#pragma once
#include "BaseComponent.h"
#include "PlatformMovementComponent.h"
#include "TimeSingleton.h"

// Attached to the player-controlled Hot Dog in Versus mode.
// Responsibilities:
//   - Kill P1 (Mr. Pepper) when within hit radius
//   - Accept a stun from P1's pepper cloud
//   - Freeze movement for the stun duration

class VersusEnemyPlayerComponent : public dae::BaseComponent
{
public:
    VersusEnemyPlayerComponent(dae::GameObject& owner,
                               PlatformMovementComponent* self,
                               PlatformMovementComponent* target)
        : BaseComponent(owner)
        , m_self{ self }
        , m_target{ target }
    {}

    void Update() override
    {
        const float dt = dae::Time::GetInstance().GetDeltaTime();

        // Count down stun
        if (m_stunTimer > 0.f)
        {
            m_stunTimer -= dt;
            return; // stunned — can't kill
        }

        if (!m_self || !m_target) return;
        if (!m_target->IsAlive()) return;

        const float dx = m_self->GetPosX() - m_target->GetPosX();
        const float dy = m_self->GetPosY() - m_target->GetPosY();
        if (dx * dx + dy * dy < HIT_RADIUS_SQ)
            m_target->Kill();
    }

    void Render() override {}

    // Called by PepperComponent when its cloud overlaps this enemy-player
    void Stun(float duration)
    {
        m_stunTimer = duration;
        if (m_self) m_self->FreezeFor(duration);
    }

    bool IsStunned() const { return m_stunTimer > 0.f; }

    float GetPosX() const { return m_self ? m_self->GetPosX() : 0.f; }
    float GetPosY() const { return m_self ? m_self->GetPosY() : 0.f; }

private:
    PlatformMovementComponent* m_self;
    PlatformMovementComponent* m_target;
    float m_stunTimer{ 0.f };

    static constexpr float HIT_RADIUS_SQ = 12.f * 12.f;
};
