#include "GridMovementComponent.h"
#include "GridRegistry.h"
#include "GameObject.h"
#include "TimeSingleton.h"

namespace dae
{
    GridMovementComponent::GridMovementComponent(GameObject& owner, int tileSize, glm::ivec2 startGridPos, glm::ivec2 gridSize, float moveSpeed, GridRegistry* registry, bool registerSelf)
        : BaseComponent(owner)
        , m_RegisterSelf{ registerSelf }
        , m_TileSize{ tileSize }
        , m_GridSize{ gridSize }
        , m_BaseSpeed{ moveSpeed }
        , m_MoveSpeed{ moveSpeed }
        , m_Registry{ registry }
        , m_GridPos{ startGridPos }
        , m_TargetGridPos{ startGridPos }
        , m_PixelPos{ float(startGridPos.x * tileSize), float(startGridPos.y * tileSize) }
    {
        GetOwner()->SetLocalPosition(m_PixelPos.x, m_PixelPos.y);
        if (m_Registry && registerSelf)
            m_Registry->Register(startGridPos, GetOwner());
    }

    void GridMovementComponent::Update()
    {
        const float dt = Time::GetInstance().GetDeltaTime();

        if (m_LockTimer > 0.f)
            m_LockTimer -= dt;

        m_BufferedDirection = { 0, 0 };

        if (!m_IsMoving)
            return;

        const glm::vec2 target{ float(m_TargetGridPos.x * m_TileSize), float(m_TargetGridPos.y * m_TileSize) };
        const glm::vec2 toTarget = target - m_PixelPos;
        const float distToTarget = glm::length(toTarget);
        const float step = m_MoveSpeed * dt;

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
        // Honour facing update always, but block movement while locked
        m_FacingDirection = direction;
        if (m_LockTimer > 0.f) return;

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
        }
    }

    void GridMovementComponent::WarpTo(glm::ivec2 cell)
    {
        if (m_Registry && m_RegisterSelf)
            m_Registry->Move(m_GridPos, cell);

        m_GridPos = cell;
        m_TargetGridPos = cell;
        m_PixelPos = { float(cell.x * m_TileSize), float(cell.y * m_TileSize) };
        m_IsMoving = false;
        m_LockTimer = 0.f;
        m_BufferedDirection = { 0, 0 };
        GetOwner()->SetLocalPosition(m_PixelPos.x, m_PixelPos.y);
    }
}
