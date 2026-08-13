#pragma once
#include "BaseComponent.h"
#include "TimeSingleton.h"
#include "InputManager.h"
#include "Font.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>

// Overlay component  attach to a late-added GameObject in the level scene.
// Pressing P or gamepad Start toggles pause.  While paused:
//   • GetDeltaTime() returns 0 (all movement/timers freeze automatically)
//   • A semi-transparent overlay + text is drawn on top of the game.
class PauseComponent : public dae::BaseComponent
{
public:
    PauseComponent(dae::GameObject& owner,
                   std::shared_ptr<dae::Font> bigFont,
                   std::shared_ptr<dae::Font> smallFont)
        : BaseComponent(owner)
        , m_bigFont{ std::move(bigFont) }
        , m_smallFont{ std::move(smallFont) }
    {}

    // Make sure pause is cleared when the component is destroyed (scene change).
    ~PauseComponent() override { dae::Time::GetInstance().SetPaused(false); }

    void Update() override
    {
        const auto* keys  = SDL_GetKeyboardState(nullptr);
        auto& input       = dae::InputManager::GetInstance();

        const bool pauseDown = keys[SDL_SCANCODE_P]
            || input.IsGamepadButtonHeld(dae::GamepadButton::Start, 0)
            || input.IsGamepadButtonHeld(dae::GamepadButton::Start, 1);

        if (pauseDown && !m_prevPause)
        {
            m_paused = !m_paused;
            dae::Time::GetInstance().SetPaused(m_paused);
        }
        m_prevPause = pauseDown;
    }

    void Render() override
    {
        if (!m_paused) return;

        SDL_Renderer* r = dae::Renderer::GetInstance().GetSDLRenderer();

        // Semi-transparent dark overlay over the whole screen
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 140);
        SDL_FRect full{ 0.f, 0.f, 1024.f, 576.f };
        SDL_RenderFillRect(r, &full);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);

        DrawTextCentered(r, m_bigFont->GetFont(),   "PAUSE",                512.f, 240.f, { 255, 255, 255, 255 });
        DrawTextCentered(r, m_smallFont->GetFont(), "P / START to resume",  512.f, 310.f, { 180, 180, 180, 255 });
    }

private:
    std::shared_ptr<dae::Font> m_bigFont;
    std::shared_ptr<dae::Font> m_smallFont;
    bool m_paused  { false };
    bool m_prevPause{ false };

    static void DrawTextCentered(SDL_Renderer* r, TTF_Font* font,
                                  const char* text, float cx, float cy, SDL_Color col)
    {
        SDL_Surface* s = TTF_RenderText_Blended(font, text, SDL_strlen(text), col);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        if (t)
        {
            SDL_FRect dst{ cx - s->w * 0.5f, cy - s->h * 0.5f,
                           static_cast<float>(s->w), static_cast<float>(s->h) };
            SDL_RenderTexture(r, t, nullptr, &dst);
            SDL_DestroyTexture(t);
        }
        SDL_DestroySurface(s);
    }
};
