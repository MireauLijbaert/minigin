#pragma once
#include "BaseComponent.h"
#include <glm/glm.hpp>

namespace dae
{
    class GridRegistry;

    class GridMovementComponent : public BaseComponent
    {
    public:
        GridMovementComponent(GameObject& owner, int tileSize, glm::ivec2 startGridPos, glm::ivec2 gridSize, float moveSpeed, GridRegistry* registry = nullptr, bool registerSelf = true);

        void Update() override;
        void Render() override {}

        // Called by input commands
        void SetDirection(glm::ivec2 direction);

        void Respawn(glm::ivec2 spawnCell);

        glm::ivec2 GetGridPos() const { return m_GridPos; }
        glm::ivec2 GetTargetGridPos() const { return m_TargetGridPos; }
        glm::ivec2 GetFacingDirection() const { return m_FacingDirection; }
        glm::ivec2 GetGridSize() const { return m_GridSize; }
        GridRegistry* GetRegistry() const { return m_Registry; }
        bool IsMoving() const { return m_IsMoving; }

    protected:
        virtual bool CanMoveTo(glm::ivec2 targetGridPos) const;

    private:
        void TryMove(glm::ivec2 direction);

        bool m_RegisterSelf;
        int m_TileSize;
        glm::ivec2 m_GridSize;
        float m_MoveSpeed;
        GridRegistry* m_Registry;

        glm::ivec2 m_FacingDirection{ 0, 1 };
        glm::ivec2 m_GridPos;
        glm::ivec2 m_TargetGridPos;
        glm::vec2 m_PixelPos;
        bool m_IsMoving{ false };
        glm::ivec2 m_BufferedDirection{ 0, 0 };
    };
}
