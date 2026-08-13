#pragma once
#include "BaseComponent.h"
#include "InputManager.h"
#include "TimeSingleton.h"
#include <functional>
#include <SDL3/SDL.h>

class GameOverScreenComponent : public dae::BaseComponent
{
public:
    GameOverScreenComponent(dae::GameObject& owner, std::function<void()> onRestart)
        : BaseComponent(owner)
        , m_onRestart{ std::move(onRestart) }
    {}

    void Render() override {}

    void Update() override
    {
        if (m_inputDelay > 0.f)
        {
            m_inputDelay -= dae::Time::GetInstance().GetDeltaTime();
            return;
        }

        const auto* keys = SDL_GetKeyboardState(nullptr);
        bool pressed = keys[SDL_SCANCODE_RETURN] != 0 || keys[SDL_SCANCODE_R] != 0
            || dae::InputManager::GetInstance().IsGamepadButtonHeld(dae::GamepadButton::Start, 0);
        if (pressed && !m_prevKey)
        {
            m_prevKey = true;
            m_onRestart();
        }
        m_prevKey = pressed;
    }

private:
    std::function<void()> m_onRestart;
    bool  m_prevKey   { false };
    float m_inputDelay{ 0.4f };
};
