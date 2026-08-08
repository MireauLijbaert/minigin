#include "EnemyComponent.h"
#include "PlatformMovementComponent.h"
#include "ScoreManager.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <cmath>
#include <cstdlib>

EnemyComponent::EnemyComponent(dae::GameObject& owner,
                               const dae::LevelMap* levelMap,
                               glm::vec2 worldPos,
                               float charWorldW, float charWorldH,
                               const LevelTransform& transform,
                               PlatformMovementComponent* player,
                               EnemyType type)
    : BaseComponent(owner)
    , m_levelMap{ levelMap }
    , m_player{ player }
    , m_type{ type }
    , m_posX{ worldPos.x }
    , m_posY{ worldPos.y }
    , m_charHalfW{ charWorldW * 0.5f }
    , m_charRenderH{ charWorldH + 2.f * transform.scaleY }
    , m_spawnPos{ worldPos }
    , m_speed{ SPEED_SPRITE * transform.scaleX }
    , m_platSnap{ 2.f * transform.scaleY }
    , m_ladrSnap{ 4.f * transform.scaleX }
    , m_interThresh{ 0.5f * transform.scaleX }
    , m_hitRadiusSq{ (charWorldW * 0.8f) * (charWorldW * 0.8f) }
{
    float px = m_player->GetPosX();
    m_MovementDirection = { (px >= m_posX) ? 1.f : -1.f, 0.f };

    const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_platSnap * 2.f);
    if (plat)
    {
        m_posY = plat->y;
        m_state = State::Walking;
    }
    else
    {
        // Off-screen spawn, enter from the side
        m_state = State::Entering;
    }

    SyncPosition();
}


void EnemyComponent::Stun()
{
    if (m_state != State::Walking) return;
    m_state = State::Stunned;
    m_stateTimer = STUN_DURATION;
    ScoreManager::GetInstance().AddScore(100);
}

void EnemyComponent::CatchByBurger()
{
    if (m_state == State::Dead || m_state == State::FallingWithBurger) return;
    m_state = State::FallingWithBurger;
}

void EnemyComponent::SetFallingY(float y)
{
    m_posY = y;
    SyncPosition();
}

void EnemyComponent::Kill()
{
    if (m_state == State::Dead) return;
    m_state = State::Dead;
    m_stateTimer = RESPAWN_DELAY;
    m_intersectionCooldown = 0.f;
    GetOwner()->SetLocalPosition(-2000.f, -2000.f);
}

void EnemyComponent::Reset(float delay)
{
    m_posX = m_spawnPos.x;
    m_posY = m_spawnPos.y;
    m_intersectionCooldown = 0.f;

    if (delay > 0.f)
    {
        m_stateTimer = delay;
        m_state = State::Waiting;
    }
    else
    {
        m_stateTimer = 0.f;
        // Check if spawn is off-screen (no platform at spawn X)
        const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_platSnap * 2.f);
        m_state = plat ? State::Walking : State::Entering;
        if (plat) m_posY = plat->y;
    }
    SyncPosition();
}

void EnemyComponent::Update()
{
    float dt = dae::Time::GetInstance().GetDeltaTime();

    switch (m_state)
    {
    case State::Entering:
    {
        m_posX += m_MovementDirection.x * m_speed * dt;
        const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_platSnap * 2.f);
        if (plat)
        {
            m_posY = plat->y;
            m_state = State::Walking;
        }
        SyncPosition();
        break;
    }

    case State::Walking:
        // Check collision with player first
        if (m_player->IsAlive())
        {
            float dx = m_posX - m_player->GetPosX();
            float dy = m_posY - m_player->GetPosY();
            if (dx * dx + dy * dy < m_hitRadiusSq)
                m_player->Kill();
        }
        UpdateWalking(dt);
        break;

    case State::Stunned:
        m_stateTimer -= dt;
        if (m_stateTimer <= 0.f)
            m_state = State::Walking;
        break;

    case State::FallingWithBurger:
        // Position is driven externally by BurgerPieceComponent::SetFallingY()
        break;

    case State::Dead:
        m_stateTimer -= dt;
        if (m_stateTimer <= 0.f)
        {
            m_posX = m_spawnPos.x;
            m_posY = m_spawnPos.y;
            m_MovementDirection = { (m_player->GetPosX() >= m_posX) ? 1.f : -1.f, 0.f };
            const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_platSnap * 2.f);
            m_state = plat ? State::Walking : State::Entering;
            if (plat) m_posY = plat->y;
            SyncPosition();
        }
        break;

    case State::Waiting:
        m_stateTimer -= dt;
        if (m_stateTimer <= 0.f && m_player->IsAlive())
        {
            m_MovementDirection = { (m_player->GetPosX() >= m_posX) ? 1.f : -1.f, 0.f };
            const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_platSnap * 2.f);
            m_state = plat ? State::Walking : State::Entering;
            if (plat) m_posY = plat->y;
        }
        break;
    }
}

void EnemyComponent::UpdateWalking(float dt)
{
    if (m_intersectionCooldown > 0.f) m_intersectionCooldown -= dt;

    float px = m_player->GetPosX();
    float py = m_player->GetPosY();

    float nextX = m_posX + m_MovementDirection.x * m_speed * dt;
    float nextY = m_posY - m_MovementDirection.y * m_speed * dt;

    bool edgeReached = false;
    if (m_MovementDirection.x != 0.f)
    {
        const auto* plat = m_levelMap->FindPlatform(nextX, m_posY, m_platSnap);
        if (plat)
        {
            m_posX = nextX;
            m_posY = plat->y;
        }
        else edgeReached = true;
    }
    else if (m_MovementDirection.y != 0.f)
    {
        const auto* ladr = m_levelMap->FindLadder(m_posX, nextY, m_ladrSnap);
        if (ladr)
            m_posY = nextY;
        else edgeReached = true;
    }

    if (edgeReached)
    {
        if (m_MovementDirection.x != 0.f)
        {
            const auto* ladr = m_levelMap->FindLadder(m_posX, m_posY, m_interThresh);
            if (ladr)
            {
                bool canUp   = m_posY > ladr->y0;
                bool canDown = m_posY < ladr->y1;
                if      (canUp   && py < m_posY) m_MovementDirection = { 0,  1 };
                else if (canDown && py > m_posY) m_MovementDirection = { 0, -1 };
                else if (canUp)                   m_MovementDirection = { 0,  1 };
                else if (canDown)                 m_MovementDirection = { 0, -1 };
                m_posX = ladr->x;
                m_intersectionCooldown = INTERSECTION_COOLDOWN;
            }
            else m_MovementDirection.x = -m_MovementDirection.x;
        }
        else
        {
            const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_platSnap);
            if (plat)
            {
                m_posY = plat->y;
                m_MovementDirection = { (px >= m_posX) ? 1.f : -1.f, 0.f };
                m_intersectionCooldown = INTERSECTION_COOLDOWN;
            }
        }
        SyncPosition();
        return;
    }

    if (m_intersectionCooldown > 0.f)
    {
        SyncPosition();
        return;
    }

    bool atIntersection = false;
    if (m_MovementDirection.x != 0.f)
    {
        if (m_levelMap->FindLadder(m_posX, m_posY, m_interThresh)) atIntersection = true;
    }
    else if (m_MovementDirection.y != 0.f)
    {
        if (m_levelMap->FindPlatform(m_posX, m_posY, m_interThresh)) atIntersection = true;
    }

    if (atIntersection)
    {
        bool onSameObject = false;
        if (m_MovementDirection.x != 0.f)
        {
            const auto* platform = m_levelMap->FindPlatform(m_posX, m_posY, m_platSnap);
            if (platform)
                onSameObject = std::abs(py - m_posY) < m_platSnap
                            && px >= platform->x0
                            && px <= platform->x1;
        }
        else
        {
            const auto* ladder = m_levelMap->FindLadder(m_posX, m_posY, m_ladrSnap);
            if (ladder)
                onSameObject = std::abs(px - m_posX) < m_ladrSnap
                            && py >= ladder->y0
                            && py <= ladder->y1;
        }

        if (onSameObject)
        {
            bool facingPlayer = (m_MovementDirection.x != 0.f)
                ? (m_MovementDirection.x > 0.f) == (px > m_posX)
                : (m_MovementDirection.y > 0.f) == (py < m_posY);

            if (facingPlayer)
            {
                m_intersectionCooldown = INTERSECTION_COOLDOWN;
                if ((rand() % 3) == 0)
                {
                    if (m_MovementDirection.x != 0.f)
                    {
                        const auto* ladr = m_levelMap->FindLadder(m_posX, m_posY, m_interThresh);
                        if (ladr)
                        {
                            bool canUp   = m_posY > ladr->y0;
                            bool canDown = m_posY < ladr->y1;
                            if (canUp && canDown)
                                m_MovementDirection = (rand() % 2) == 0 ? glm::vec2(0, 1) : glm::vec2(0, -1);
                            else if (canUp)   m_MovementDirection = { 0,  1 };
                            else if (canDown) m_MovementDirection = { 0, -1 };
                            m_posX = ladr->x;
                        }
                    }
                    else
                    {
                        const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_interThresh);
                        if (plat)
                        {
                            bool canLeft  = m_posX > plat->x0;
                            bool canRight = m_posX < plat->x1;
                            if (canLeft && canRight)
                                m_MovementDirection = (rand() % 2) == 0 ? glm::vec2(-1, 0) : glm::vec2(1, 0);
                            else if (canLeft)  m_MovementDirection = { -1, 0 };
                            else if (canRight) m_MovementDirection = {  1, 0 };
                            m_posY = plat->y;
                        }
                    }
                }
            }
        }
        else
        {
            if (m_MovementDirection.x != 0.f)
            {
                const auto* ladr = m_levelMap->FindLadder(m_posX, m_posY, m_interThresh);
                if (ladr)
                {
                    bool canUp   = m_posY > ladr->y0;
                    bool canDown = m_posY < ladr->y1;
                    if (canUp && py < m_posY)
                    {
                        m_MovementDirection = { 0, 1 };
                        m_posX = ladr->x;
                        m_intersectionCooldown = INTERSECTION_COOLDOWN;
                    }
                    else if (canDown && py > m_posY)
                    {
                        m_MovementDirection = { 0, -1 };
                        m_posX = ladr->x;
                        m_intersectionCooldown = INTERSECTION_COOLDOWN;
                    }
                }
            }
            else
            {
                const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_interThresh);
                if (plat)
                {
                    bool canRight = m_posX < plat->x1;
                    bool canLeft  = m_posX > plat->x0;
                    if (canRight && px > m_posX)
                    {
                        m_MovementDirection = { 1, 0 };
                        m_posY = plat->y;
                        m_intersectionCooldown = INTERSECTION_COOLDOWN;
                    }
                    else if (canLeft && px < m_posX)
                    {
                        m_MovementDirection = { -1, 0 };
                        m_posY = plat->y;
                        m_intersectionCooldown = INTERSECTION_COOLDOWN;
                    }
                }
            }
        }
    }

    SyncPosition();
}

void EnemyComponent::SyncPosition()
{
    GetOwner()->SetLocalPosition(m_posX - m_charHalfW, m_posY - m_charRenderH);
}
