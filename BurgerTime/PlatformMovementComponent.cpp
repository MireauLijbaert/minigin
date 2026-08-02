#include "PlatformMovementComponent.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>

PlatformMovementComponent::PlatformMovementComponent(dae::GameObject& owner,
                                                     const dae::LevelMap* levelMap,
                                                     glm::vec2 worldPos,
                                                     float charWorldW, float charWorldH,
                                                     const LevelTransform& transform)
    : BaseComponent(owner)
    , m_levelMap{ levelMap }
    , m_posX{ worldPos.x }
    , m_posY{ worldPos.y }
    , m_charHalfW{ charWorldW * 0.5f }
    , m_charRenderH{ charWorldH + 2.f * transform.scaleY }
    , m_stepX{ transform.scaleX }
    , m_stepY{ transform.scaleY }
    , m_platThresh{ 2.f * transform.scaleY }
    , m_ladrThresh{ 4.f * transform.scaleX }
{
    SyncPosition();
}

void PlatformMovementComponent::Update()
{
    const float dt = dae::Time::GetInstance().GetDeltaTime();
    const auto* keys = SDL_GetKeyboardState(nullptr);

    const bool anyKey = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_RIGHT]
                     || keys[SDL_SCANCODE_UP]   || keys[SDL_SCANCODE_DOWN];

    if (!anyKey)
    {
        m_stepTimer = STEP_INTERVAL;
        SyncPosition();
        return;
    }

    m_stepTimer += dt;
    if (m_stepTimer < STEP_INTERVAL)
    {
        SyncPosition();
        return;
    }
    m_stepTimer = 0.f;

    // Vertical takes priority: only if a ladder is reachable
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_DOWN])
    {
        float dy = keys[SDL_SCANCODE_DOWN] ? m_stepY : -m_stepY;
        float newY = m_posY + dy;
        const auto* ladr = m_levelMap->FindLadder(m_posX, newY, m_ladrThresh);
        if (ladr)
        {
            m_posY = newY;
            m_posX = ladr->x;
            SyncPosition();
            return;
        }
    }

    // Horizontal: only if on a platform
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_RIGHT])
    {
        float dx = keys[SDL_SCANCODE_RIGHT] ? m_stepX : -m_stepX;
        float newX = m_posX + dx;
        const auto* plat = m_levelMap->FindPlatform(newX, m_posY, m_platThresh);
        if (plat)
        {
            m_posX = newX;
            m_posY = plat->y;
        }
    }

    SyncPosition();
}

void PlatformMovementComponent::SyncPosition()
{
    GetOwner()->SetLocalPosition(m_posX - m_charHalfW, m_posY - m_charRenderH);
}
