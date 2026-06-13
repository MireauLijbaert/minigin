#pragma once
#include "SoundSystem.h"
#include <memory>

namespace dae {

    class ServiceLocator {
    public:
        static SoundSystem& GetSoundSystem() {
            return s_SoundSystem ? *s_SoundSystem : s_NullSoundSystem;
        }

        static void RegisterSoundSystem(std::unique_ptr<SoundSystem> system) {
            s_SoundSystem = std::move(system);
        }

        static void Shutdown() {
            s_SoundSystem.reset();
        }

    private:
        inline static std::unique_ptr<SoundSystem> s_SoundSystem;
        inline static NullSoundSystem s_NullSoundSystem;
    };
}
