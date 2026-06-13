#include "IceBlockComponent.h"
#include "GridRegistry.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <glm/glm.hpp>

namespace dae
{
    IceBlockComponent* IceBlockComponent::s_SlidingBlock{ nullptr };

    IceBlockComponent::IceBlockComponent(GameObject& owner, glm::ivec2 gridPos, int tileSize, float slideSpeed)
        : BaseComponent(owner)
        , m_GridPos{ gridPos }
        , m_PixelPos{ float(gridPos.x * tileSize), float(gridPos.y * tileSize) }
        , m_TargetPixelPos{ m_PixelPos }
        , m_TileSize{ tileSize }
        , m_SlideSpeed{ slideSpeed }
    {
    }

    IceBlockComponent::~IceBlockComponent()
    {
        if (s_SlidingBlock == this)
            s_SlidingBlock = nullptr;
    }

    bool IceBlockComponent::TryPush(glm::ivec2 direction, GridRegistry* registry, glm::ivec2 gridSize)
    {
        if (m_IsSliding) return false;

        // Check if there's room to slide at all
        const glm::ivec2 firstStep = m_GridPos + direction;
        if (!InBounds(firstStep, gridSize) || !registry->IsEmpty(firstStep))
        {
            // No room: block breaks
            registry->Unregister(m_GridPos);
            GetOwner()->MarkForRemoval();
            return true;
        }

        // Slide all the way until blocked
        glm::ivec2 finalPos = firstStep;
        while (true)
        {
            const glm::ivec2 next = finalPos + direction;
            if (!InBounds(next, gridSize) || !registry->IsEmpty(next))
                break;
            finalPos = next;
        }

        // Record every cell the block will pass through for kill detection
        m_SlidePath.clear();
        for (glm::ivec2 cell = firstStep; cell != finalPos + direction; cell += direction)
            m_SlidePath.push_back(cell);

        // Update registry immediately so collision is correct during animation
        registry->Move(m_GridPos, finalPos);
        m_GridPos = finalPos;
        m_TargetPixelPos = { float(finalPos.x * m_TileSize), float(finalPos.y * m_TileSize) };
        m_IsSliding = true;
        s_SlidingBlock = this;
        return true;
    }

    void IceBlockComponent::Update()
    {
        if (!m_IsSliding) return;

        const glm::vec2 toTarget = m_TargetPixelPos - m_PixelPos;
        const float dist = glm::length(toTarget);
        const float step = m_SlideSpeed * Time::GetInstance().GetDeltaTime();

        if (dist <= step)
        {
            m_PixelPos = m_TargetPixelPos;
            m_IsSliding = false;
            m_SlidePath.clear();
            s_SlidingBlock = nullptr;
        }
        else
        {
            m_PixelPos += (toTarget / dist) * step;
        }

        GetOwner()->SetLocalPosition(m_PixelPos.x, m_PixelPos.y);
    }

    bool IceBlockComponent::IsInSlidePath(glm::ivec2 cell) const
    {
        for (const auto& c : m_SlidePath)
            if (c == cell) return true;
        return false;
    }

    bool IceBlockComponent::InBounds(glm::ivec2 pos, glm::ivec2 gridSize)
    {
        return pos.x >= 0 && pos.x < gridSize.x && pos.y >= 0 && pos.y < gridSize.y;
    }
}
