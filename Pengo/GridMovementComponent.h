#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class GridMovementComponent : public BaseComponent
    {
    public:
        GridMovementComponent(GameObject& owner, int tileSize, glm::ivec2 startGridPos, glm::ivec2 gridSize, float moveSpeed);

        void Update() override;
        void Render() override {}

        // Called by input commands
        void SetDirection(glm::ivec2 direction);

        glm::ivec2 GetGridPos() const { return m_GridPos; }
        bool IsMoving() const { return m_IsMoving; }

    protected:
        virtual bool CanMoveTo(glm::ivec2 targetGridPos) const;

    private:
        void TryMove(glm::ivec2 direction);

        int m_TileSize;
        glm::ivec2 m_GridSize;
        float m_MoveSpeed;

        glm::ivec2 m_GridPos;
        glm::ivec2 m_TargetGridPos;
        glm::vec2 m_PixelPos;
        bool m_IsMoving{ false };
        glm::ivec2 m_BufferedDirection{ 0, 0 };
    };
}
