#include "SnoBeeComponent.h"
#include "SnoBeeStates.h"
#include "GridMovementComponent.h"
#include "GridRegistry.h"
#include "IceBlockComponent.h"
#include "HealthComponent.h"
#include "GameObject.h"

namespace dae
{
    SnoBeeComponent::SnoBeeComponent(GameObject& owner, glm::ivec2 gridSize, GridRegistry* registry, GameObject* player)
        : BaseComponent(owner)
        , m_Player{ player }
        , m_GridSize{ gridSize }
        , m_Registry{ registry }
    {
        m_StateMachine.SetState(std::make_unique<SnoBeeWanderState>(*this));
    }

    void SnoBeeComponent::Update()
    {
        if (m_Dead) return;

        m_StateMachine.Update();

        // Apply any queued transition after the state has finished its Update
        if (m_PendingState)
            m_StateMachine.SetState(std::move(m_PendingState));

        CheckPlayerCollision();
        CheckBlockCollision();
    }

    void SnoBeeComponent::CheckPlayerCollision()
    {
        if (!m_Player) return;

        auto* myMov = GetMovement();
        auto* playerMov = m_Player->GetComponent<GridMovementComponent>();
        if (!myMov || !playerMov) return;

        if (myMov->GetGridPos() == playerMov->GetGridPos())
        {
            if (auto* health = m_Player->GetComponent<HealthComponent>())
                health->LoseLife();
        }
    }

    void SnoBeeComponent::RequestTransition(std::unique_ptr<BaseState> newState)
    {
        m_PendingState = std::move(newState);
    }

    void SnoBeeComponent::Stun(float duration)
    {
        if (m_Dead) return;
        m_StateMachine.SetState(std::make_unique<SnoBeeStunnedState>(*this, duration));
    }

    void SnoBeeComponent::CheckBlockCollision()
    {
        auto* myMov = GetMovement();
        if (!myMov) return;

        const glm::ivec2 myCell = myMov->GetGridPos();
        const glm::ivec2 myTarget = myMov->GetTargetGridPos();

        if (auto* block = IceBlockComponent::GetSlidingBlock())
        {
            // Forgiving kill zone: die if current cell OR target cell (mid-move) is in path
            if (block->IsInSlidePath(myCell) || block->IsInSlidePath(myTarget))
                Die();
        }
    }

    void SnoBeeComponent::Die()
    {
        if (m_Dead) return;
        m_Dead = true;
        // Sno-bees use registerSelf=false so they are NOT in the registry, no Unregister needed
        GetOwner()->MarkForRemoval();
    }

    GridMovementComponent* SnoBeeComponent::GetMovement()
    {
        if (!m_Movement)
            m_Movement = GetOwner()->GetComponent<GridMovementComponent>();
        return m_Movement;
    }
}
