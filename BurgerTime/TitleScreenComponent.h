#pragma once
#include "BaseComponent.h"
#include "InputManager.h"
#include "TextComponent.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>
#include <functional>
#include <string>

// Title screen mode selector.
// D-pad up/down or keyboard arrows navigate; A / Start / Enter confirms.
// Shortcut keys: 1 = 1P, 2 = CO-OP, 3 = VS.
class TitleScreenComponent : public dae::BaseComponent
{
public:
    TitleScreenComponent(dae::GameObject& owner,
                         dae::TextComponent* opt1Text,    // updated with cursor prefix
                         dae::TextComponent* opt2Text,
                         dae::TextComponent* opt3Text,
                         dae::TextComponent* promptText,  // blinking bottom line
                         std::function<void()> onStart1P,
                         std::function<void()> onStart2P,
                         std::function<void()> onStartVS)
        : BaseComponent(owner)
        , m_optTexts{ opt1Text, opt2Text, opt3Text }
        , m_promptText{ promptText }
        , m_callbacks{ std::move(onStart1P), std::move(onStart2P), std::move(onStartVS) }
    {
        UpdateLabels();
    }

    void Update() override
    {
        if (m_started) return;

        const float dt = dae::Time::GetInstance().GetDeltaTime();
        auto& input    = dae::InputManager::GetInstance();
        const auto* keys = SDL_GetKeyboardState(nullptr);

        // ── Startup delay: ignore all input briefly so held keys don't bleed ──
        if (m_inputDelay > 0.f)
        {
            m_inputDelay -= dt;
            return;
        }

        // ── Blink prompt ─────────────────────────────────────────────────
        m_blinkTimer -= dt;
        if (m_blinkTimer <= 0.f)
        {
            m_blinkTimer   = BLINK_INTERVAL;
            m_blinkVisible = !m_blinkVisible;
            if (m_promptText)
                m_promptText->SetText(m_blinkVisible ? PROMPT : "");
        }

        // ── Navigate up ──────────────────────────────────────────────────
        const bool navUp = keys[SDL_SCANCODE_UP] != 0
                        || input.IsGamepadButtonHeld(dae::GamepadButton::DPadUp, 0);
        if (navUp && !m_prevUp)
        {
            m_selected = (m_selected + NUM_OPTIONS - 1) % NUM_OPTIONS;
            UpdateLabels();
        }
        m_prevUp = navUp;

        // ── Navigate down ────────────────────────────────────────────────
        const bool navDown = keys[SDL_SCANCODE_DOWN] != 0
                          || input.IsGamepadButtonHeld(dae::GamepadButton::DPadDown, 0);
        if (navDown && !m_prevDown)
        {
            m_selected = (m_selected + 1) % NUM_OPTIONS;
            UpdateLabels();
        }
        m_prevDown = navDown;

        // ── Confirm ──────────────────────────────────────────────────────
        const bool confirm = keys[SDL_SCANCODE_RETURN] != 0
                          || keys[SDL_SCANCODE_SPACE]  != 0
                          || input.IsGamepadButtonHeld(dae::GamepadButton::A,     0)
                          || input.IsGamepadButtonHeld(dae::GamepadButton::Start, 0);
        if (confirm && !m_prevConfirm)
        {
            m_started = true;
            m_callbacks[m_selected]();
        }
        m_prevConfirm = confirm;

        // ── Shortcut keys 1 / 2 / 3 ─────────────────────────────────────
        const bool k1 = keys[SDL_SCANCODE_1] != 0;
        if (k1 && !m_prev1 && !m_started) { m_started = true; m_callbacks[0](); }
        m_prev1 = k1;

        const bool k2 = keys[SDL_SCANCODE_2] != 0;
        if (k2 && !m_prev2 && !m_started) { m_started = true; m_callbacks[1](); }
        m_prev2 = k2;

        const bool k3 = keys[SDL_SCANCODE_3] != 0;
        if (k3 && !m_prev3 && !m_started) { m_started = true; m_callbacks[2](); }
        m_prev3 = k3;
    }

    void Render() override {}

private:
    static constexpr int NUM_OPTIONS = 3;
    static constexpr const char* OPTION_NAMES[NUM_OPTIONS] = { "1 PLAYER", "CO-OP", "VS" };
    static constexpr float       BLINK_INTERVAL = 0.5f;
    static constexpr const char* PROMPT = "PUSH START  /  A TO SELECT";

    dae::TextComponent*   m_optTexts[NUM_OPTIONS];
    dae::TextComponent*   m_promptText;
    std::function<void()> m_callbacks[NUM_OPTIONS];

    int  m_selected   { 0 };
    bool m_started    { false };
    bool m_prevUp     { false };
    bool m_prevDown   { false };
    bool m_prevConfirm{ false };
    bool m_prev1      { false };
    bool m_prev2      { false };
    bool m_prev3      { false };
    bool  m_blinkVisible{ true };
    float m_blinkTimer  { BLINK_INTERVAL };
    float m_inputDelay  { 0.4f }; // ignore input on first load to avoid button bleed

    void UpdateLabels()
    {
        for (int i = 0; i < NUM_OPTIONS; ++i)
        {
            if (!m_optTexts[i]) continue;
            std::string label = (i == m_selected) ? "> " : "  ";
            label += OPTION_NAMES[i];
            m_optTexts[i]->SetText(label);
        }
    }
};
