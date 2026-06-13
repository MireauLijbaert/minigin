#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae { class GridRegistry; }

namespace dae
{
    class IceBlockComponent : public BaseComponent
    {
    public:
        IceBlockComponent(GameObject& owner, glm::ivec2 gridPos, int tileSize, float slideSpeed = 256.f);

        void Update() override;
        void Render() override {}

        // Called by the pusher (e.g. Pengo) when it moves into this block's cell.
        // Returns true if the block can be and was pushed (registry updated immediately, animation starts).
        bool TryPush(glm::ivec2 direction, GridRegistry* registry, glm::ivec2 gridSize);

        glm::ivec2 GetGridPos() const { return m_GridPos; }
        bool IsSliding() const { return m_IsSliding; }

    private:
        static bool InBounds(glm::ivec2 pos, glm::ivec2 gridSize);

        glm::ivec2 m_GridPos;
        glm::vec2 m_PixelPos;
        glm::vec2 m_TargetPixelPos;
        int m_TileSize;
        float m_SlideSpeed;
        bool m_IsSliding{ false };
    };
}
