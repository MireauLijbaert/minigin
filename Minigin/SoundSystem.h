#pragma once
#include <string>
#include <memory>

namespace dae {
    class SoundSystem {
    public:
        virtual ~SoundSystem() = default;
        virtual void Play(const std::string& filename, int volume) = 0;
        virtual void PlayMusic(const std::string& filename, int loops = -1) = 0;
        virtual void StopMusic() = 0;
        virtual void SetMusicVolume(int volume) = 0;
        virtual void Update() = 0;
    };

    class SdlSoundSystem final : public SoundSystem {
    public:
        SdlSoundSystem();
        ~SdlSoundSystem();
        void Play(const std::string& filename, int volume) override;
        void PlayMusic(const std::string& filename, int loops = -1) override;
        void StopMusic() override;
        void SetMusicVolume(int volume) override;
        void Update() override;
    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };

    class NullSoundSystem final : public SoundSystem {
    public:
        void Play(const std::string&, int) override {}
        void PlayMusic(const std::string&, int) override {}
        void StopMusic() override {}
        void SetMusicVolume(int) override {}
        void Update() override {}
    };
}