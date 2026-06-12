#include "GridMovementComponent.h"
#include "GameObject.h"
#include "TimeSingleton.h"

namespace dae
{
    GridMovementComponent::GridMovementComponent(GameObject& owner, int tileSize, glm::ivec2 startGridPos, glm::ivec2 gridSize, float moveSpeed)
        : BaseComponent(owner)
        , m_TileSize{ tileSize }
        , m_GridSize{ gridSize }
        , m_MoveSpeed{ moveSpeed }
        , m_GridPos{ startGridPos }
        , m_TargetGridPos{ startGridPos }
        , m_PixelPos{ float(startGridPos.x * tileSize), float(startGridPos.y * tileSize) }
    {
        GetOwner()->SetLocalPosition(m_PixelPos.x, m_PixelPos.y);
    }

    void GridMovementComponent::Update()
    {
        m_BufferedDirection = { 0, 0 };

        if (!m_IsMoving)
            return;

        const glm::vec2 target{ float(m_TargetGridPos.x * m_TileSize), float(m_TargetGridPos.y * m_TileSize) };
        const glm::vec2 toTarget = target - m_PixelPos;
        const float distToTarget = glm::length(toTarget);
        const float step = m_MoveSpeed * Time::GetInstance().GetDeltaTime();

        if (distToTarget <= step)
        {
            // Arrived, snap exactly to grid cell
            m_PixelPos = target;
            m_GridPos = m_TargetGridPos;
            m_IsMoving = false;

            // Apply buffered input if any
            if (m_BufferedDirection != glm::ivec2{ 0, 0 })
            {
                TryMove(m_BufferedDirection);
                m_BufferedDirection = { 0, 0 };
            }
        }
        else
        {
            m_PixelPos += (toTarget / distToTarget) * step;
        }

        GetOwner()->SetLocalPosition(m_PixelPos.x, m_PixelPos.y);
    }

    void GridMovementComponent::SetDirection(glm::ivec2 direction)
    {
        if (!m_IsMoving)
            TryMove(direction);
        else
            m_BufferedDirection = direction;
    }

    bool GridMovementComponent::CanMoveTo(glm::ivec2 targetGridPos) const
    {
        return targetGridPos.x >= 0 && targetGridPos.x < m_GridSize.x
            && targetGridPos.y >= 0 && targetGridPos.y < m_GridSize.y;
    }

    void GridMovementComponent::TryMove(glm::ivec2 direction)
    {
        const glm::ivec2 target = m_GridPos + direction;
        if (CanMoveTo(target))
        {
            m_TargetGridPos = target;
            m_IsMoving = true;
        }
    }
}
