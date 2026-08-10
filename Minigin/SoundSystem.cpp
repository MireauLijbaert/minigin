#include "SoundSystem.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <unordered_map>
#include <vector>
#include <queue>
#include <variant>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <string>
#include <iostream>
#include <algorithm>

namespace dae {

    // ---------------------------------------------------------------------------
    // Request types pushed onto the queue by the game thread
    // ---------------------------------------------------------------------------
    struct PlaySoundRequest        { std::string filename; int volume; };
    struct PlayMusicRequest        { std::string filename; int loops; };
    struct PlayMusicAfterRequest   { std::string musicFilename; std::string sfxFilename; int sfxVolume; int loops; };
    struct StopMusicRequest        {};
    struct SetMusicVolRequest      { int volume; };
    struct SetMuteRequest          { bool muted; };

    using AudioRequest = std::variant<
        PlaySoundRequest,
        PlayMusicRequest,
        PlayMusicAfterRequest,
        StopMusicRequest,
        SetMusicVolRequest,
        SetMuteRequest
    >;

    // ---------------------------------------------------------------------------
    // Impl - owns the mixer, all cached audio, and the worker thread
    // ---------------------------------------------------------------------------
    struct SdlSoundSystem::Impl {
        MIX_Mixer* mixer{ nullptr };

        // Audio cache: keyed by filename, shared between SFX and music
        std::unordered_map<std::string, MIX_Audio*> audioCache;

        // Active one-shot SFX tracks (created per play, destroyed when done)
        std::vector<MIX_Track*> sfxTracks;

        // Dedicated track for background music
        MIX_Track* musicTrack{ nullptr };

        // Set when PlayMusicAfter is used: monitor this SFX track, then start pendingMusicFile
        MIX_Track* pendingSfxTrack{ nullptr };
        std::string pendingMusicFile;
        int pendingMusicLoops{ -1 };

        // Last music request — remembered so we can restart it on unmute.
        std::string m_lastMusicFile;
        int         m_lastMusicLoops{ -1 };

        // Thread-safe queue
        std::queue<AudioRequest> queue;
        std::mutex mutex;
        std::condition_variable cv;
        std::thread worker;
        std::atomic<bool> running{ true };
        std::atomic<bool> m_muted{ false };

        Impl() {
            MIX_Init();
            mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
            if (!mixer)
                std::cerr << "SoundSystem: MIX_CreateMixerDevice failed: " << SDL_GetError() << "\n";
            worker = std::thread([this]() { WorkerLoop(); });
        }

        ~Impl() {
            running = false;
            cv.notify_one();
            if (worker.joinable()) worker.join();

            // Free SFX tracks
            for (MIX_Track* t : sfxTracks)
                MIX_DestroyTrack(t);

            // Free pending SFX track (PlayMusicAfter)
            if (pendingSfxTrack) MIX_DestroyTrack(pendingSfxTrack);

            // Free music track
            if (musicTrack) MIX_DestroyTrack(musicTrack);

            // Free cached audio objects
            for (auto& [key, audio] : audioCache)
                MIX_DestroyAudio(audio);

            if (mixer) MIX_DestroyMixer(mixer);
            MIX_Quit();
        }

        // Called by game thread - non-blocking
        void Push(AudioRequest req) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                queue.push(std::move(req));
            }
            cv.notify_one();
        }

        // Worker thread entry point
        void WorkerLoop() {
            while (true) {
                std::unique_lock<std::mutex> lock(mutex);

                // When waiting for an SFX to finish, poll every 50ms instead of sleeping forever
                if (pendingSfxTrack)
                    cv.wait_for(lock, std::chrono::milliseconds(50), [this] { return !queue.empty() || !running; });
                else
                    cv.wait(lock, [this] { return !queue.empty() || !running; });

                // Check if the awaited SFX has finished — if so, start the music
                if (pendingSfxTrack && !MIX_TrackPlaying(pendingSfxTrack))
                {
                    MIX_DestroyTrack(pendingSfxTrack);
                    pendingSfxTrack = nullptr;
                    lock.unlock();
                    Handle(PlayMusicRequest{ pendingMusicFile, pendingMusicLoops });
                    lock.lock();
                }

                while (!queue.empty()) {
                    AudioRequest req = std::move(queue.front());
                    queue.pop();
                    lock.unlock();

                    CleanupFinishedTracks();
                    Process(req);

                    lock.lock();
                }

                if (!running) break;
            }
        }

        // Remove tracks whose audio has finished playing
        void CleanupFinishedTracks() {
            sfxTracks.erase(
                std::remove_if(sfxTracks.begin(), sfxTracks.end(), [](MIX_Track* t) {
                    if (!MIX_TrackPlaying(t)) {
                        MIX_DestroyTrack(t);
                        return true;
                    }
                    return false;
                }),
                sfxTracks.end()
            );
        }

        void Process(const AudioRequest& req) {
            std::visit([this](auto&& r) { Handle(r); }, req);
        }

        // Load (or return cached) MIX_Audio. predecode=true for SFX, false for music.
        MIX_Audio* GetAudio(const std::string& filename, bool predecode) {
            if (!mixer) return nullptr;
            auto it = audioCache.find(filename);
            if (it != audioCache.end()) return it->second;

            MIX_Audio* audio = MIX_LoadAudio(mixer, filename.c_str(), predecode);
            if (!audio)
                std::cerr << "SoundSystem: failed to load '" << filename << "': " << SDL_GetError() << "\n";
            else
                audioCache[filename] = audio;
            return audio;
        }

        void Handle(const PlaySoundRequest& r) {
            if (!mixer) return;
            if (m_muted) return;   // suppress all SFX while muted
            MIX_Audio* audio = GetAudio(r.filename, true);
            if (!audio) return;

            MIX_Track* track = MIX_CreateTrack(mixer);
            if (!track) {
                // Fallback: fire-and-forget, no volume control
                MIX_PlayAudio(mixer, audio);
                return;
            }

            MIX_SetTrackAudio(track, audio);
            MIX_SetTrackGain(track, r.volume / 128.0f);
            MIX_PlayTrack(track, 0);
            sfxTracks.push_back(track);
        }

        void Handle(const SetMuteRequest& r) {
            m_muted = r.muted;

            if (r.muted) {
                // Stop the intro jingle (PlayMusicAfter pending SFX).
                if (pendingSfxTrack) {
                    MIX_StopTrack(pendingSfxTrack, 0);
                    MIX_DestroyTrack(pendingSfxTrack);
                    pendingSfxTrack = nullptr;
                    pendingMusicFile.clear();
                }
                // Stop BGM.
                if (musicTrack) {
                    MIX_StopTrack(musicTrack, 0);
                    MIX_DestroyTrack(musicTrack);
                    musicTrack = nullptr;
                }
                // Stop any playing SFX.
                for (MIX_Track* t : sfxTracks)
                    MIX_StopTrack(t, 0);
            } else {
                // Unmute: restart the last known music from the beginning.
                if (!m_lastMusicFile.empty())
                    Handle(PlayMusicRequest{ m_lastMusicFile, m_lastMusicLoops });
            }
        }

        void Handle(const PlayMusicAfterRequest& r) {
            if (!mixer) return;

            // Always remember so unmute can replay the full sequence.
            m_lastMusicFile  = r.musicFilename;
            m_lastMusicLoops = r.loops;

            // Don't play anything while muted.
            if (m_muted) return;

            // Cancel any previous pending music-after-sfx
            if (pendingSfxTrack) {
                MIX_DestroyTrack(pendingSfxTrack);
                pendingSfxTrack = nullptr;
            }

            MIX_Audio* sfxAudio = GetAudio(r.sfxFilename, true);
            if (!sfxAudio) return;

            MIX_Track* track = MIX_CreateTrack(mixer);
            if (!track) return;

            MIX_SetTrackAudio(track, sfxAudio);
            MIX_SetTrackGain(track, r.sfxVolume / 128.0f);
            MIX_PlayTrack(track, 0);

            // Store: worker loop polls this and starts music when it finishes
            pendingSfxTrack   = track;
            pendingMusicFile  = r.musicFilename;
            pendingMusicLoops = r.loops;
        }

        void Handle(const PlayMusicRequest& r) {
            if (!mixer) return;

            // Always remember so unmute can replay it.
            m_lastMusicFile  = r.filename;
            m_lastMusicLoops = r.loops;

            // Don't play while muted.
            if (m_muted) return;

            MIX_Audio* next = GetAudio(r.filename, false);
            if (!next) return;

            // Skip if already playing this exact audio
            if (musicTrack && MIX_GetTrackAudio(musicTrack) == next && MIX_TrackPlaying(musicTrack))
                return;

            // Destroy old track and create a fresh one
            if (musicTrack) {
                MIX_DestroyTrack(musicTrack);
                musicTrack = nullptr;
            }

            musicTrack = MIX_CreateTrack(mixer);
            if (!musicTrack) return;

            MIX_SetTrackAudio(musicTrack, next);

            SDL_PropertiesID props = SDL_CreateProperties();
            SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, (Sint64)r.loops);
            MIX_PlayTrack(musicTrack, props);
            SDL_DestroyProperties(props);
        }

        void Handle(const StopMusicRequest&) {
            if (musicTrack) MIX_StopTrack(musicTrack, 0);
        }

        void Handle(const SetMusicVolRequest& r) {
            if (musicTrack) MIX_SetTrackGain(musicTrack, r.volume / 128.0f);
        }
    };

    // ---------------------------------------------------------------------------
    // SdlSoundSystem public API - all calls are non-blocking
    // ---------------------------------------------------------------------------
    SdlSoundSystem::SdlSoundSystem()
        : pImpl{ std::make_unique<Impl>() }
    {}

    SdlSoundSystem::~SdlSoundSystem()
    {
        pImpl.reset();
    }

    void SdlSoundSystem::Play(const std::string& filename, int volume)
    {
        if (filename.empty()) return;
        pImpl->Push(PlaySoundRequest{ filename, volume });
    }

    void SdlSoundSystem::PlayMusic(const std::string& filename, int loops)
    {
        pImpl->Push(PlayMusicRequest{ filename, loops });
    }

    void SdlSoundSystem::PlayMusicAfter(const std::string& musicFile, const std::string& sfxFile, int sfxVolume, int loops)
    {
        if (sfxFile.empty()) return;
        pImpl->Push(PlayMusicAfterRequest{ musicFile, sfxFile, sfxVolume, loops });
    }

    void SdlSoundSystem::StopMusic()
    {
        pImpl->Push(StopMusicRequest{});
    }

    void SdlSoundSystem::SetMusicVolume(int volume)
    {
        pImpl->Push(SetMusicVolRequest{ volume });
    }

    void SdlSoundSystem::SetMuted(bool muted)
    {
        pImpl->Push(SetMuteRequest{ muted });
        pImpl->m_muted = muted;   // also update the atomic immediately so IsMuted() is coherent
    }

    bool SdlSoundSystem::IsMuted() const
    {
        return pImpl->m_muted.load();
    }

    // Update() is a no-op: the worker thread processes the queue independently.
    void SdlSoundSystem::Update() {}

}
