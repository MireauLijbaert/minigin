#pragma once
#include "BaseComponent.h"
#include "StateMachine.h"
#include <glm/glm.hpp>
#include <memory>

class BaseState;

namespace dae
{
    class GridRegistry;
    class GridMovementComponent;
    class GameObject;

    class SnoBeeComponent final : public BaseComponent
    {
    public:
        SnoBeeComponent(GameObject& owner, glm::ivec2 gridSize, GridRegistry* registry, GameObject* player);

        void Update() override;
        void Render() override {}

        void RequestTransition(std::unique_ptr<BaseState> newState);

        void Stun(float duration = 2.f);
        void Die();

        GridMovementComponent* GetMovement();
        glm::ivec2 GetGridSize() const { return m_GridSize; }
        GridRegistry* GetRegistry() const { return m_Registry; }
        GameObject* GetPlayer() const { return m_Player; }

    private:
        void CheckPlayerCollision();
        void CheckBlockCollision();

        GridMovementComponent* m_Movement{ nullptr };
        StateMachine m_StateMachine;
        std::unique_ptr<BaseState> m_PendingState;

        GameObject* m_Player;
        glm::ivec2 m_GridSize;
        GridRegistry* m_Registry;
        bool m_Dead{ false };
    };
}
