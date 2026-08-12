#pragma once
#include "BaseComponent.h"
#include "LevelMap.h"
#include "LevelLoader.h"
#include "Renderer.h"
#include <vector>
#include <SDL3/SDL.h>

class DebugLevelComponent : public dae::BaseComponent
{
public:
    DebugLevelComponent(dae::GameObject& owner,
                        const dae::LevelMap* levelMap,
                        const std::vector<CupDef>* cups,
                        const LevelTransform& transform)
        : BaseComponent(owner)
        , m_levelMap{ levelMap }
        , m_cups{ cups }
        , m_transform{ transform }
    {}

    void Update() override
    {
        const auto* keys = SDL_GetKeyboardState(nullptr);
        bool pressed = keys[SDL_SCANCODE_F3] != 0;
        if (pressed && !m_prevKey)
            m_visible = !m_visible;
        m_prevKey = pressed;
    }

    void Render() override
    {
        if (!m_visible) return;

        SDL_Renderer* r = dae::Renderer::GetInstance().GetSDLRenderer();
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

        // Platforms: cyan
        SDL_SetRenderDrawColor(r, 0, 255, 255, 180);
        for (const auto& p : m_levelMap->GetPlatforms())
        {
            SDL_FRect rect{ p.x0, p.y - 1.f, p.x1 - p.x0, 3.f };
            SDL_RenderFillRect(r, &rect);
        }

        // Ladders: green
        SDL_SetRenderDrawColor(r, 0, 255, 80, 180);
        for (const auto& l : m_levelMap->GetLadders())
        {
            SDL_FRect rect{ l.x - 1.f, l.y0, 3.f, l.y1 - l.y0 };
            SDL_RenderFillRect(r, &rect);
        }

        // Cups: yellow bracket (two vertical bars)
        SDL_SetRenderDrawColor(r, 255, 220, 0, 200);
        if (m_cups)
        {
            for (const auto& cup : *m_cups)
            {
                float cx = m_transform.WorldX(static_cast<float>(GridLadderX(cup.col)));
                float cy = m_transform.WorldY(static_cast<float>(GridPlatformY(cup.row)));
                float w = 24.f * m_transform.scaleX;
                float h = 8.f  * m_transform.scaleY;
                // left bar
                SDL_FRect left{ cx - w * 0.5f, cy, 3.f, h };
                SDL_RenderFillRect(r, &left);
                // right bar
                SDL_FRect right{ cx + w * 0.5f - 3.f, cy, 3.f, h };
                SDL_RenderFillRect(r, &right);
                // bottom bar
                SDL_FRect bottom{ cx - w * 0.5f, cy + h - 2.f, w, 3.f };
                SDL_RenderFillRect(r, &bottom);
            }
        }

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    }

private:
    const dae::LevelMap* m_levelMap;
    const std::vector<CupDef>* m_cups;
    LevelTransform m_transform;
    bool m_visible{ false };
    bool m_prevKey{ false };
};
