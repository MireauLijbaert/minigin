#include "EnemyComponent.h"
#include "PlatformMovementComponent.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <cmath>
#include <cstdlib>

EnemyComponent::EnemyComponent(dae::GameObject& owner,
                               const dae::LevelMap* levelMap,
                               glm::vec2 worldPos,
                               float charWorldW, float charWorldH,
                               const LevelTransform& transform,
                               PlatformMovementComponent* player)
    : BaseComponent(owner)
    , m_levelMap{ levelMap }
    , m_player{ player }
    , m_posX{ worldPos.x }
    , m_posY{ worldPos.y }
    , m_charHalfW{ charWorldW * 0.5f }
    , m_charRenderH{ charWorldH + 2.f * transform.scaleY }
    , m_speed{ SPEED_SPRITE * transform.scaleX }
    , m_platSnap{ 2.f * transform.scaleY }
    , m_ladrSnap{ 4.f * transform.scaleX }
    , m_interThresh{ 0.5f * transform.scaleX }
{
    const auto* plat = m_levelMap->FindPlatform(m_posX, m_posY, m_platSnap * 2.f);
    if (plat) m_posY = plat->y;

    float px = m_player->GetPosX();
    m_MovementDirection = { (px >= m_posX) ? 1.f : -1.f, 0.f };

    SyncPosition();
}

void EnemyComponent::Update()
{
    float dt = dae::Time::GetInstance().GetDeltaTime();
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

    // End of object: forced intersection or dead-end reverse
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
            // Rule 2: not facing player, ignore intersection
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
