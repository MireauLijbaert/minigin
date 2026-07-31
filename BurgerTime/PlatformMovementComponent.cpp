#include "PlatformMovementComponent.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>

PlatformMovementComponent::PlatformMovementComponent(dae::GameObject& owner,
                                                     const dae::LevelMap* levelMap,
                                                     glm::vec2 spritePos,
                                                     float scaleX, float scaleY,
                                                     float offsetX, float offsetY,
                                                     float spriteW, float spriteH)
    : BaseComponent(owner)
    , m_levelMap{ levelMap }
    , m_spritePosX{ spritePos.x }
    , m_spritePosY{ spritePos.y }
    , m_scaleX{ scaleX }
    , m_scaleY{ scaleY }
    , m_offsetX{ offsetX }
    , m_offsetY{ offsetY }
    , m_spriteW{ spriteW }
    , m_spriteH{ spriteH }
{
    SyncWorldPosition();
}

void PlatformMovementComponent::Update()
{
    const float dt = dae::Time::GetInstance().GetDeltaTime();
    const auto* keys = SDL_GetKeyboardState(nullptr);

    const bool anyKey = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_RIGHT]
                     || keys[SDL_SCANCODE_UP]   || keys[SDL_SCANCODE_DOWN];

    if (!anyKey)
    {
        m_stepTimer = STEP_INTERVAL; // so the next keypress steps immediately
        SyncWorldPosition();
        return;
    }

    m_stepTimer += dt;
    if (m_stepTimer < STEP_INTERVAL)
    {
        SyncWorldPosition();
        return;
    }
    m_stepTimer = 0.f;

    // Vertical takes priority: only if a ladder is reachable
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_DOWN])
    {
        float dy = keys[SDL_SCANCODE_DOWN] ? STEP : -STEP;
        float newY = m_spritePosY + dy;
        const auto* ladr = m_levelMap->FindLadder(m_spritePosX, newY, LADR_THRESHOLD);
        if (ladr)
        {
            m_spritePosY = newY;
            m_spritePosX = static_cast<float>(ladr->x); // snap center to ladder column
            SyncWorldPosition();
            return;
        }
    }

    // Horizontal: only if on a platform
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_RIGHT])
    {
        float dx = keys[SDL_SCANCODE_RIGHT] ? STEP : -STEP;
        float newX = m_spritePosX + dx;
        const auto* plat = m_levelMap->FindPlatform(newX, m_spritePosY, PLAT_THRESHOLD);
        if (plat)
        {
            m_spritePosX = newX;
            m_spritePosY = static_cast<float>(plat->y); // snap feet to platform row
        }
    }

    SyncWorldPosition();
}

void PlatformMovementComponent::SyncWorldPosition()
{
    // Sprite position is feet-center; top-left for rendering:
    float worldX = m_offsetX + (m_spritePosX - m_spriteW * 0.5f) * m_scaleX;
    float worldY = m_offsetY + (m_spritePosY - m_spriteH - 2) * m_scaleY;
    GetOwner()->SetLocalPosition(worldX, worldY);
}
