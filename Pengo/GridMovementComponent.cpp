#include "GridMovementComponent.h"
#include "IceBlockComponent.h"
#include "GridRegistry.h"
#include "GameObject.h"
#include "TimeSingleton.h"

namespace dae
{
    GridMovementComponent::GridMovementComponent(GameObject& owner, int tileSize, glm::ivec2 startGridPos, glm::ivec2 gridSize, float moveSpeed, GridRegistry* registry)
        : BaseComponent(owner)
        , m_TileSize{ tileSize }
        , m_GridSize{ gridSize }
        , m_MoveSpeed{ moveSpeed }
        , m_Registry{ registry }
        , m_GridPos{ startGridPos }
        , m_TargetGridPos{ startGridPos }
        , m_PixelPos{ float(startGridPos.x * tileSize), float(startGridPos.y * tileSize) }
    {
        GetOwner()->SetLocalPosition(m_PixelPos.x, m_PixelPos.y);
        if (m_Registry)
            m_Registry->Register(startGridPos, GetOwner());
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
            if (m_Registry)
                m_Registry->Move(m_GridPos, m_TargetGridPos);
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
        if (targetGridPos.x < 0 || targetGridPos.x >= m_GridSize.x ||
            targetGridPos.y < 0 || targetGridPos.y >= m_GridSize.y)
            return false;

        // check if something is already occupying that cell
        if (m_Registry && !m_Registry->IsEmpty(targetGridPos))
            return false;

        return true;
    }

    void GridMovementComponent::TryMove(glm::ivec2 direction)
    {
        const glm::ivec2 target = m_GridPos + direction;
        if (CanMoveTo(target))
        {
            m_TargetGridPos = target;
            m_IsMoving = true;
            return;
        }

        // If a block is in the way, try to push it
        if (m_Registry)
        {
            if (auto* obj = m_Registry->GetAt(target))
            {
                if (auto* block = obj->GetComponent<IceBlockComponent>())
                {
                    if (block->TryPush(direction, m_Registry, m_GridSize))
                    {
                        // Block pushed out of the way, Pengo steps into its old cell
                        m_TargetGridPos = target;
                        m_IsMoving = true;
                    }
                }
            }
        }
    }
}
