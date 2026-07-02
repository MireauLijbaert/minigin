#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>
#include <vector>

namespace dae { class GridRegistry; }

namespace dae
{
    class IceBlockComponent : public BaseComponent
    {
    public:
        IceBlockComponent(GameObject& owner, glm::ivec2 gridPos, int tileSize, float slideSpeed = 256.f);
        ~IceBlockComponent() override;

        void Update() override;
        void Render() override {}

        bool TryPush(glm::ivec2 direction, GridRegistry* registry, glm::ivec2 gridSize);

        glm::ivec2 GetGridPos() const { return m_GridPos; }
        bool IsSliding() const { return m_IsSliding; }

        // Slide path: all cells this block passes through during current/last slide
        bool IsInSlidePath(glm::ivec2 cell) const;

        // The one block currently mid-slide
        static IceBlockComponent* GetSlidingBlock() { return s_SlidingBlock; }

    private:
        static bool InBounds(glm::ivec2 pos, glm::ivec2 gridSize);

        glm::ivec2 m_GridPos;
        glm::vec2 m_PixelPos;
        glm::vec2 m_TargetPixelPos;
        int m_TileSize;
        float m_SlideSpeed;
        bool m_IsSliding{ false };

        std::vector<glm::ivec2> m_SlidePath;
        static IceBlockComponent* s_SlidingBlock;
    };
}
