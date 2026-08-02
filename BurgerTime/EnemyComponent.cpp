#include "EnemyComponent.h"
#include "PlatformMovementComponent.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <cmath>
#include <cstdlib>


EnemyComponent::EnemyComponent(dae::GameObject& owner,
                               const dae::LevelMap* levelMap,
                               glm::vec2 startSpritePos,
                               float scaleX, float scaleY,
                               float offsetX, float offsetY,
                               float spriteW, float spriteH,
                               PlatformMovementComponent* player)
    : BaseComponent(owner)
    , m_levelMap{ levelMap }
    , m_player{ player }
    , m_spritePosX{ startSpritePos.x }
    , m_spritePosY{ startSpritePos.y }
    , m_scaleX{ scaleX }
    , m_scaleY{ scaleY }
    , m_offsetX{ offsetX }
    , m_offsetY{ offsetY }
    , m_spriteW{ spriteW }
    , m_spriteH{ spriteH }
{
    // Snap to nearest platform and set initial direction toward player
    const auto* plat = m_levelMap->FindPlatform(m_spritePosX, m_spritePosY, 4.f);
    if (plat) m_spritePosY = static_cast<float>(plat->y);

    float px = m_player->GetSpritePosX();
    m_MovementDirection = { (px >= m_spritePosX) ? 1.f : -1.f, 0.f };

    SyncWorldPosition();
}

void EnemyComponent::Update()
{
    float dt = dae::Time::GetInstance().GetDeltaTime();
    if (m_intersectionCooldown > 0.f) m_intersectionCooldown -= dt;

    float px = m_player->GetSpritePosX();
    float py = m_player->GetSpritePosY();

    // Peek at next position before committing to the move
    float nextX = m_spritePosX + m_MovementDirection.x * SPEED * dt;
    float nextY = m_spritePosY - m_MovementDirection.y * SPEED * dt;

    bool edgeReached = false;
    if (m_MovementDirection.x != 0.f)
    {
        const auto* plat = m_levelMap->FindPlatform(nextX, m_spritePosY, PLAT_SNAP);
        if (plat)
        {
            m_spritePosX = nextX;
            m_spritePosY = static_cast<float>(plat->y);
        }
        else edgeReached = true;
    }
    else if (m_MovementDirection.y != 0.f)
    {
        const auto* ladr = m_levelMap->FindLadder(m_spritePosX, nextY, LADR_SNAP);
        if (ladr)
            m_spritePosY = nextY;
        else edgeReached = true;
    }

    // End of object: forced intersection or dead-end reverse
    if (edgeReached)
    {
        if (m_MovementDirection.x != 0.f)
        {
            const auto* ladr = m_levelMap->FindLadder(m_spritePosX, m_spritePosY, INTER_THRESH);
            if (ladr)
            {
                bool canUp   = m_spritePosY > static_cast<float>(ladr->y0);
                bool canDown = m_spritePosY < static_cast<float>(ladr->y1);
                if      (canUp   && py < m_spritePosY) m_MovementDirection = { 0,  1 };
                else if (canDown && py > m_spritePosY) m_MovementDirection = { 0, -1 };
                else if (canUp)                         m_MovementDirection = { 0,  1 };
                else if (canDown)                       m_MovementDirection = { 0, -1 };
                m_spritePosX = static_cast<float>(ladr->x);
                m_intersectionCooldown = INTERSECTION_COOLDOWN;
            }
            else m_MovementDirection.x = -m_MovementDirection.x; // true dead-end
        }
        else
        {
            const auto* plat = m_levelMap->FindPlatform(m_spritePosX, m_spritePosY, PLAT_SNAP);
            if (plat)
            {
                m_spritePosY = static_cast<float>(plat->y);
                m_MovementDirection = { (px >= m_spritePosX) ? 1.f : -1.f, 0.f };
                m_intersectionCooldown = INTERSECTION_COOLDOWN;
            }
        }
        SyncWorldPosition();
        return;
    }

    // Optional intersection check, skip while cooldown is active
    if (m_intersectionCooldown > 0.f)
    {
        SyncWorldPosition();
        return;
    }

    bool atIntersection = false;
    if (m_MovementDirection.x != 0.f)
    {
        const dae::LadderCol* ladder = m_levelMap->FindLadder(m_spritePosX, m_spritePosY, INTER_THRESH);
        if (ladder) atIntersection = true;
    }
    else if (m_MovementDirection.y != 0.f)
    {
        const dae::PlatformRow* platform = m_levelMap->FindPlatform(m_spritePosX, m_spritePosY, INTER_THRESH);
        if (platform) atIntersection = true;
    }

    if (atIntersection)
    {
        bool onSameObject = false;
        if (m_MovementDirection.x != 0.f)
        {
            const auto* platform = m_levelMap->FindPlatform(m_spritePosX, m_spritePosY, PLAT_SNAP);
            if (platform)
                onSameObject = std::abs(py - m_spritePosY) < PLAT_SNAP
                            && px >= static_cast<float>(platform->x0)
                            && px <= static_cast<float>(platform->x1);
        }
        else
        {
            const auto* ladder = m_levelMap->FindLadder(m_spritePosX, m_spritePosY, LADR_SNAP);
            if (ladder)
                onSameObject = std::abs(px - m_spritePosX) < LADR_SNAP
                            && py >= static_cast<float>(ladder->y0)
                            && py <= static_cast<float>(ladder->y1);
        }

        if (onSameObject)
        {
            bool facingPlayer = (m_MovementDirection.x != 0.f)
                ? (m_MovementDirection.x > 0.f) == (px > m_spritePosX)
                : (m_MovementDirection.y > 0.f) == (py < m_spritePosY);

            if (facingPlayer)
            {
                // Rule 1: roll once per crossing, cooldown is set regardless of outcome
                m_intersectionCooldown = INTERSECTION_COOLDOWN;
                if ((rand() % 3) == 0)
                {
                    if (m_MovementDirection.x != 0.f)
                    {
                        const auto* ladr = m_levelMap->FindLadder(m_spritePosX, m_spritePosY, INTER_THRESH);
                        if (ladr)
                        {
                            bool canUp   = m_spritePosY > static_cast<float>(ladr->y0);
                            bool canDown = m_spritePosY < static_cast<float>(ladr->y1);
                            if (canUp && canDown)
                                m_MovementDirection = (rand() % 2) == 0 ? glm::vec2(0, 1) : glm::vec2(0, -1);
                            else if (canUp)   m_MovementDirection = { 0,  1 };
                            else if (canDown) m_MovementDirection = { 0, -1 };
                            m_spritePosX = static_cast<float>(ladr->x);
                        }
                    }
                    else
                    {
                        const auto* plat = m_levelMap->FindPlatform(m_spritePosX, m_spritePosY, INTER_THRESH);
                        if (plat)
                        {
                            bool canLeft  = m_spritePosX > static_cast<float>(plat->x0);
                            bool canRight = m_spritePosX < static_cast<float>(plat->x1);
                            if (canLeft && canRight)
                                m_MovementDirection = (rand() % 2) == 0 ? glm::vec2(-1, 0) : glm::vec2(1, 0);
                            else if (canLeft)  m_MovementDirection = { -1, 0 };
                            else if (canRight) m_MovementDirection = {  1, 0 };
                            m_spritePosY = static_cast<float>(plat->y);
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
                const auto* ladr = m_levelMap->FindLadder(m_spritePosX, m_spritePosY, INTER_THRESH);
                if (ladr)
                {
                    bool canUp   = m_spritePosY > static_cast<float>(ladr->y0);
                    bool canDown = m_spritePosY < static_cast<float>(ladr->y1);
                    if (canUp && py < m_spritePosY)
                    {
                        m_MovementDirection = { 0, 1 };
                        m_spritePosX = static_cast<float>(ladr->x);
                        m_intersectionCooldown = INTERSECTION_COOLDOWN;
                    }
                    else if (canDown && py > m_spritePosY)
                    {
                        m_MovementDirection = { 0, -1 };
                        m_spritePosX = static_cast<float>(ladr->x);
                        m_intersectionCooldown = INTERSECTION_COOLDOWN;
                    }
                }
            }
            else
            {
                const auto* plat = m_levelMap->FindPlatform(m_spritePosX, m_spritePosY, INTER_THRESH);
                if (plat)
                {
                    bool canRight = m_spritePosX < static_cast<float>(plat->x1);
                    bool canLeft  = m_spritePosX > static_cast<float>(plat->x0);
                    if (canRight && px > m_spritePosX)
                    {
                        m_MovementDirection = { 1, 0 };
                        m_spritePosY = static_cast<float>(plat->y);
                        m_intersectionCooldown = INTERSECTION_COOLDOWN;
                    }
                    else if (canLeft && px < m_spritePosX)
                    {
                        m_MovementDirection = { -1, 0 };
                        m_spritePosY = static_cast<float>(plat->y);
                        m_intersectionCooldown = INTERSECTION_COOLDOWN;
                    }
                }
            }
        }
    }

    SyncWorldPosition();
}

void EnemyComponent::SyncWorldPosition()
{
    float worldX = m_offsetX + (m_spritePosX - m_spriteW * 0.5f) * m_scaleX;
    float worldY = m_offsetY + (m_spritePosY - m_spriteH - 2.f)  * m_scaleY;
    GetOwner()->SetLocalPosition(worldX, worldY);
}
