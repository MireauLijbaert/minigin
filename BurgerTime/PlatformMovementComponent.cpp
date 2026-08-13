#include "PlatformMovementComponent.h"
#include "EnemyComponent.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include "Event.h"
#include "InputManager.h"
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
    if (m_state != PlayerState::Alive) return;
    m_state = PlayerState::Dying;
    m_isMoving = false;
    m_deathTimer = DEATH_ANIM_DURATION;
    if (m_health) m_health->LoseLife();
    m_subject.NotifyObservers(dae::Event("PlayerDied"), GetOwner());

    // Enemies wait until after death anim + respawn delay
    if (m_enemies)
    {
        float stagger = 0.f;
        for (auto* enemy : *m_enemies)
        {
            enemy->Reset(DEATH_ANIM_DURATION + RESPAWN_DELAY + stagger);
            stagger += 0.5f;
        }
    }
}

void PlatformMovementComponent::Update()
{
    const float dt = dae::Time::GetInstance().GetDeltaTime();

    if (m_state == PlayerState::Dying)
    {
        m_deathTimer -= dt;
        if (m_deathTimer <= 0.f)
        {
            // Animation done — hide player and start respawn countdown
            GetOwner()->SetLocalPosition(-2000.f, -2000.f);
            m_state = PlayerState::Dead;
            m_respawnTimer = RESPAWN_DELAY;
        }
        return;
    }

    if (m_state == PlayerState::Dead)
    {
        m_respawnTimer -= dt;
        if (m_respawnTimer <= 0.f && (!m_health || m_health->IsAlive()))
        {
            m_posX = m_startPos.x;
            m_posY = m_startPos.y;
            m_state = PlayerState::Alive;
            m_invincibleTimer = INVINCIBLE_DURATION;
            SyncPosition();
            m_subject.NotifyObservers(dae::Event("PlayerRespawned"), GetOwner());
        }
        return;
    }

    // Tick post-respawn invincibility window
    if (m_invincibleTimer > 0.f)
        m_invincibleTimer -= dt;

    // Pepper throw freeze: block movement input for the throw duration
    if (m_freezeTimer > 0.f)
    {
        m_freezeTimer -= dt;
        m_isMoving = false;
        SyncPosition();
        return;
    }

    const auto* keys = SDL_GetKeyboardState(nullptr);

    auto& input = dae::InputManager::GetInstance();
    const bool gpUp    = m_useGamepad && input.IsGamepadButtonHeld(dae::GamepadButton::DPadUp,    m_gamepadIndex);
    const bool gpDown  = m_useGamepad && input.IsGamepadButtonHeld(dae::GamepadButton::DPadDown,  m_gamepadIndex);
    const bool gpLeft  = m_useGamepad && input.IsGamepadButtonHeld(dae::GamepadButton::DPadLeft,  m_gamepadIndex);
    const bool gpRight = m_useGamepad && input.IsGamepadButtonHeld(dae::GamepadButton::DPadRight, m_gamepadIndex);

    const bool wUp    = keys[m_keys.up]    || gpUp;
    const bool wDown  = keys[m_keys.down]  || gpDown;
    const bool wLeft  = keys[m_keys.left]  || gpLeft;
    const bool wRight = keys[m_keys.right] || gpRight;

    if (!wUp && !wDown && !wLeft && !wRight)
    {
        m_isMoving = false;
        m_stepTimer = m_stepInterval;
        SyncPosition();
        return;
    }
    m_isMoving = true;

    m_stepTimer += dt;
    if (m_stepTimer < m_stepInterval)
    {
        SyncPosition();
        return;
    }
    m_stepTimer = 0.f;

    // Vertical takes priority: only if a ladder is reachable
    if (wUp || wDown)
    {
        float dy = wDown ? m_stepY : -m_stepY;
        float newY = m_posY + dy;
        const auto* ladr = m_levelMap->FindLadder(m_posX, newY, m_ladrThresh);
        if (ladr)
        {
            m_posY = newY;
            m_posX = ladr->x;
            m_facingDir = { 0.f, wDown ? -1.f : 1.f };
            SyncPosition();
            return;
        }
    }

    // Horizontal: only if on a platform
    if (wLeft || wRight)
    {
        float dx = wRight ? m_stepX : -m_stepX;
        float newX = m_posX + dx;
        const auto* plat = m_levelMap->FindPlatform(newX, m_posY, m_platThresh);
        if (plat)
        {
            m_posX = newX;
            m_posY = plat->y;
            m_facingDir = { wRight ? 1.f : -1.f, 0.f };
        }
    }

    SyncPosition();
}

void PlatformMovementComponent::SyncPosition()
{
    GetOwner()->SetLocalPosition(m_posX - m_charHalfW, m_posY - m_charRenderH);
}
