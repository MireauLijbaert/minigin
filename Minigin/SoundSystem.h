#include <string>
#include <memory>

namespace dae {
    class SoundSystem {
    public:
        virtual ~SoundSystem() = default;
        virtual void Play(const std::string& filename, int volume) = 0;
        virtual void Update() = 0; // Processes the queue
    };

    class SdlSoundSystem final : public SoundSystem {
    public:
        SdlSoundSystem();
        ~SdlSoundSystem();
        void Play(const std::string& filename, int volume) override;
        void Update() override;
    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };

    class NullSoundSystem final : public SoundSystem {
    public:
        void Play(const std::string&, int) override {}
        void Update() override {}
    };
}