#include "PlatformMovementComponent.h"
#include "EnemyComponent.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>

PlatformMovementComponent::PlatformMovementComponent(dae::GameObject& owner,
                                                     const dae::LevelMap* levelMap,
                                                     glm::vec2 worldPos,
                                                     float charWorldW, float charWorldH,
                                                     const LevelTransform& transform)
    : BaseComponent(owner)
    , m_startPos{ worldPos }
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

void PlatformMovementComponent::Kill()
{
    if (m_state == PlayerState::Dead) return;
    m_state = PlayerState::Dead;
    m_respawnTimer = RESPAWN_DELAY;
    if (m_health) m_health->LoseLife();
    GetOwner()->SetLocalPosition(-2000.f, -2000.f);

    if (m_enemies)
    {
        float stagger = 0.f;
        for (auto* enemy : *m_enemies)
        {
            enemy->Reset(RESPAWN_DELAY + stagger);
            stagger += 0.5f;
        }
    }
}

void PlatformMovementComponent::Update()
{
    const float dt = dae::Time::GetInstance().GetDeltaTime();

    if (m_state == PlayerState::Dead)
    {
        m_respawnTimer -= dt;
        if (m_respawnTimer <= 0.f && (!m_health || m_health->IsAlive()))
        {
            m_posX = m_startPos.x;
            m_posY = m_startPos.y;
            m_state = PlayerState::Alive;
            SyncPosition();
        }
        return;
    }

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
            m_facingDir = { 0.f, keys[SDL_SCANCODE_DOWN] ? -1.f : 1.f };
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
            m_facingDir = { keys[SDL_SCANCODE_RIGHT] ? 1.f : -1.f, 0.f };
        }
    }

    SyncPosition();
}

void PlatformMovementComponent::SyncPosition()
{
    GetOwner()->SetLocalPosition(m_posX - m_charHalfW, m_posY - m_charRenderH);
}
