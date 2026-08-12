#pragma once
#include "BaseComponent.h"
#include "TextComponent.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>
#include <functional>

// Handles the title screen: blinks "PRESS ENTER TO START" and fires onStart when
// the player presses Enter or Space.

class TitleScreenComponent : public dae::BaseComponent
{
public:
    TitleScreenComponent(dae::GameObject& owner,
                         dae::TextComponent* blinkText,
                         std::function<void()> onStart)
        : BaseComponent(owner)
        , m_blinkText{ blinkText }
        , m_onStart{ std::move(onStart) }
    {}

    void Update() override
    {
        if (m_started) return;

        // Blink the prompt label
        m_blinkTimer -= dae::Time::GetInstance().GetDeltaTime();
        if (m_blinkTimer <= 0.f)
        {
            m_blinkTimer = BLINK_INTERVAL;
            m_blinkVisible = !m_blinkVisible;
            if (m_blinkText)
                m_blinkText->SetText(m_blinkVisible ? "PRESS ENTER TO START" : "");
        }

        // Edge-triggered Enter / Space detection
        const auto* keys = SDL_GetKeyboardState(nullptr);
        const bool  down = keys[SDL_SCANCODE_RETURN] != 0 || keys[SDL_SCANCODE_SPACE] != 0;
        if (down && !m_prevDown)
        {
            m_started = true;
            m_onStart();
        }
        m_prevDown = down;
    }

    void Render() override {}

private:
    dae::TextComponent*   m_blinkText;
    std::function<void()> m_onStart;
    bool  m_started     { false };
    bool  m_prevDown    { false };
    bool  m_blinkVisible{ true };
    float m_blinkTimer  { BLINK_INTERVAL };

    static constexpr float BLINK_INTERVAL = 0.5f;
};
