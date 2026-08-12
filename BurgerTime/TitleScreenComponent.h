#pragma once
#include "BaseComponent.h"
#include "InputManager.h"
#include "TextComponent.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>
#include <functional>

// Title screen: blinks mode prompt and calls onStart1P or onStart2P.
// Press 1 / Enter / Space → 1 player
// Press 2               → 2 players (co-op)

class TitleScreenComponent : public dae::BaseComponent
{
public:
    TitleScreenComponent(dae::GameObject& owner,
                         dae::TextComponent* blinkText,
                         std::function<void()> onStart1P,
                         std::function<void()> onStart2P = {},
                         std::function<void()> onStartVS = {})
        : BaseComponent(owner)
        , m_blinkText{ blinkText }
        , m_onStart1P{ std::move(onStart1P) }
        , m_onStart2P{ std::move(onStart2P) }
        , m_onStartVS{ std::move(onStartVS) }
    {}

    void Update() override
    {
        if (m_started) return;

        m_blinkTimer -= dae::Time::GetInstance().GetDeltaTime();
        if (m_blinkTimer <= 0.f)
        {
            m_blinkTimer = BLINK_INTERVAL;
            m_blinkVisible = !m_blinkVisible;
            if (m_blinkText)
                m_blinkText->SetText(m_blinkVisible ? PROMPT : "");
        }

        const auto* keys = SDL_GetKeyboardState(nullptr);

        // Versus: press 3
        const bool down3 = keys[SDL_SCANCODE_3] != 0;
        if (down3 && !m_prev3 && m_onStartVS)
        {
            m_started = true;
            m_onStartVS();
        }
        m_prev3 = down3;

        // 2-player co-op: press 2
        const bool down2 = keys[SDL_SCANCODE_2] != 0;
        if (down2 && !m_prev2 && m_onStart2P && !m_started)
        {
            m_started = true;
            m_onStart2P();
        }
        m_prev2 = down2;

        // 1-player: press 1, Enter, Space, or gamepad Start
        const bool gpStart = dae::InputManager::GetInstance()
            .IsGamepadButtonHeld(dae::GamepadButton::Start, 0);
        const bool down1 = keys[SDL_SCANCODE_1]      != 0
                        || keys[SDL_SCANCODE_RETURN]  != 0
                        || keys[SDL_SCANCODE_SPACE]   != 0
                        || gpStart;
        if (down1 && !m_prev1 && !m_started)
        {
            m_started = true;
            m_onStart1P();
        }
        m_prev1 = down1;
    }

    void Render() override {}

private:
    dae::TextComponent*   m_blinkText;
    std::function<void()> m_onStart1P;
    std::function<void()> m_onStart2P;
    std::function<void()> m_onStartVS;
    bool  m_started    { false };
    bool  m_prev1      { false };
    bool  m_prev2      { false };
    bool  m_prev3      { false };
    bool  m_blinkVisible{ true };
    float m_blinkTimer  { BLINK_INTERVAL };

    static constexpr float       BLINK_INTERVAL = 0.5f;
    static constexpr const char* PROMPT = "1 - 1P    2 - CO-OP    3 - VS";
};
