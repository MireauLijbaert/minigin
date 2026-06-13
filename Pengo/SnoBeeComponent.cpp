#include "SnoBeeComponent.h"
#include "SnoBeeStates.h"
#include "GridMovementComponent.h"
#include "GridRegistry.h"
#include "GameManager.h"
#include "IceBlockComponent.h"
#include "HealthComponent.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "PengoSounds.h"

namespace dae
{
    SnoBeeComponent::SnoBeeComponent(GameObject& owner, glm::ivec2 gridSize, GridRegistry* registry, GameObject* player, GameManager* gameManager)
        : BaseComponent(owner)
        , m_Player{ player }
        , m_GridSize{ gridSize }
        , m_Registry{ registry }
        , m_GameManager{ gameManager }
    {
        m_StateMachine.SetState(std::make_unique<SnoBeeWanderState>(*this));
    }

    void SnoBeeComponent::Update()
    {
        if (m_Dead) return;

        ApplyFrenzySpeedIfNeeded();

        m_StateMachine.Update();

        if (m_PendingState)
            m_StateMachine.SetState(std::move(m_PendingState));

        CheckPlayerCollision();
        CheckBlockCollision();
    }

    void SnoBeeComponent::ApplyFrenzySpeedIfNeeded()
    {
        if (m_FrenzySpeedApplied) return;
        if (!IsFrenzy()) return;

        m_FrenzySpeedApplied = true;
        if (auto* mov = GetMovement())
            mov->SetSpeedMultiplier(1.5f); // frenzy: 50% faster
    }

    void SnoBeeComponent::CheckPlayerCollision()
    {
        if (!m_Player) return;

        auto* myMov = GetMovement();
        auto* playerMov = m_Player->GetComponent<GridMovementComponent>();
        if (!myMov || !playerMov) return;

        if (myMov->GetGridPos() == playerMov->GetGridPos())
        {
            dae::ServiceLocator::GetSoundSystem().Play(PengoSounds::PLAYER_HIT, PengoSounds::FULL_VOLUME);
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
        dae::ServiceLocator::GetSoundSystem().Play(PengoSounds::BEE_STUNNED, PengoSounds::FULL_VOLUME);
        m_StateMachine.SetState(std::make_unique<SnoBeeStunnedState>(*this, duration));
    }

    void SnoBeeComponent::CheckBlockCollision()
    {
        auto* myMov = GetMovement();
        if (!myMov) return;

        const glm::ivec2 myCell   = myMov->GetGridPos();
        const glm::ivec2 myTarget = myMov->GetTargetGridPos();

        if (auto* block = IceBlockComponent::GetSlidingBlock())
        {
            if (block->IsInSlidePath(myCell) || block->IsInSlidePath(myTarget))
                Die();
        }
    }

    void SnoBeeComponent::Die()
    {
        if (m_Dead) return;
        m_Dead = true;
        dae::ServiceLocator::GetSoundSystem().Play(PengoSounds::BEE_SQUASHED, PengoSounds::FULL_VOLUME);
        if (m_GameManager) m_GameManager->OnSnoBeeKilled(this);
        GetOwner()->MarkForRemoval();
    }

    void SnoBeeComponent::Respawn(glm::ivec2 spawnCell)
    {
        m_Dead = false;
        m_FrenzySpeedApplied = false;

        m_StateMachine.SetState(std::make_unique<SnoBeeWanderState>(*this));

        if (auto* mov = GetMovement())
        {
            mov->SetSpeedMultiplier(1.f); // reset to base speed
            mov->WarpTo(spawnCell);
        }
    }

    float SnoBeeComponent::GetBreakDuration() const
    {
        if (m_GameManager)
            return m_GameManager->GetBreakDuration();
        return 0.5f;
    }

    bool SnoBeeComponent::IsFrenzy() const
    {
        return m_GameManager && m_GameManager->IsFrenzy();
    }

    bool SnoBeeComponent::CanChase() const
    {
        if (!m_GameManager) return true;
        return m_GameManager->CanStartChasing();
    }

    void SnoBeeComponent::NotifyChaseStart()
    {
        if (m_GameManager) m_GameManager->RegisterChaseStart();
    }

    void SnoBeeComponent::NotifyChaseEnd()
    {
        if (m_GameManager) m_GameManager->RegisterChaseEnd();
    }

    GridMovementComponent* SnoBeeComponent::GetMovement()
    {
        if (!m_Movement)
            m_Movement = GetOwner()->GetComponent<GridMovementComponent>();
        return m_Movement;
    }
}
