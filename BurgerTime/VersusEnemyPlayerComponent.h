#pragma once
#include "BaseComponent.h"
#include "PlatformMovementComponent.h"
#include "LevelMap.h"
#include "TimeSingleton.h"
#include <glm/glm.hpp>

// Attached to the player-controlled Hot Dog in Versus mode.
//
// State machine:
//   Entering    - auto-walks horizontally from the S-marker until it lands on a platform
//                 (identical to how AI enemies enter the level)
//   WalkingIn   - brief freeze on the platform before P2 gains control
//   Active      - P2 has control; kills P1 on contact, can be stunned/burger-crushed
//   Dead        - burger crushed P2; hidden, waiting to respawn
//   LockedP1Dead- P1 died; P2 hides until P1 respawns, then re-enters

class VersusEnemyPlayerComponent : public dae::BaseComponent
{
public:
    VersusEnemyPlayerComponent(dae::GameObject& owner,
                               PlatformMovementComponent*  self,
                               PlatformMovementComponent*  target,
                               glm::vec2                   entryPos,
                               const dae::LevelMap*        levelMap,
                               float                       entrySpeed,   // SPEED_SPRITE * scaleX
                               float                       platSnap)     // 2.f * scaleY
        : BaseComponent(owner)
        , m_self{ self }
        , m_target{ target }
        , m_entryPos{ entryPos }
        , m_levelMap{ levelMap }
        , m_entrySpeed{ entrySpeed }
        , m_platSnap{ platSnap }
    {
        m_entryX = entryPos.x;
        if (m_self) m_self->SetPosition(entryPos.x, entryPos.y);
        // Walk toward P1, same logic as EnemyComponent::Entering
        if (m_target)
            m_enterDir = (m_target->GetPosX() >= entryPos.x) ? 1.f : -1.f;
    }

    void Update() override
    {
        const float dt = dae::Time::GetInstance().GetDeltaTime();

        const bool p1Alive = m_target && m_target->IsAlive();
        const bool p1Dead  = m_target && !m_target->IsAlive() && !m_target->IsDying();

        switch (m_state)
        {
        // ── Auto-walk from S-marker to the first platform ────────────────
        case VState::Entering:
            if (p1Dead) { HideAndWait(); return; }
            // Keep PlatformMovement's input locked; we drive position ourselves
            if (m_self) m_self->FreezeFor(1.f);
            m_entryX += m_enterDir * m_entrySpeed * dt;
            if (m_self) m_self->SetPosition(m_entryX, m_entryPos.y);
            if (m_levelMap)
            {
                const auto* plat = m_levelMap->FindPlatform(m_entryX, m_entryPos.y, m_platSnap * 2.f);
                if (plat)
                {
                    // Landed on a platform — brief freeze then hand over control
                    if (m_self) m_self->SetPosition(m_entryX, plat->y);
                    m_lockTimer = LOCK_ON_LAND;
                    m_state     = VState::WalkingIn;
                }
            }
            break;

        // ── Short freeze after landing, before P2 takes control ──────────
        case VState::WalkingIn:
            if (p1Dead) { HideAndWait(); return; }
            if (m_self) m_self->FreezeFor(1.f);
            m_lockTimer -= dt;
            if (m_lockTimer <= 0.f)
            {
                if (m_self) m_self->FreezeFor(0.f);
                m_state = VState::Active;
            }
            break;

        // ── Climbing out of cup ──────────────────────────────────────────
        case VState::ClimbingFromCup:
        {
            if (p1Dead) { HideAndWait(); return; }
            if (m_self) m_self->FreezeFor(1.f);
            float nextY = GetPosY() - m_entrySpeed * dt;
            if (m_self) m_self->SetPosition(GetPosX(), nextY);
            if (m_levelMap)
            {
                const auto* plat = m_levelMap->FindPlatform(GetPosX(), nextY, m_platSnap * 2.f);
                if (plat)
                {
                    if (m_self) m_self->SetPosition(GetPosX(), plat->y);
                    if (m_self) m_self->FreezeFor(0.f);
                    m_state = VState::Active;
                }
            }
            break;
        }

        // ── Active — P2 in control ───────────────────────────────────────
        case VState::Active:
            // Being carried by a falling burger — BurgerPieceComponent drives our Y
            if (m_caughtByBurger)
            {
                if (m_self) m_self->FreezeFor(1.f);
                if (p1Dead) { m_caughtByBurger = false; HideAndWait(); }
                return;
            }
            if (p1Dead) { HideAndWait(); return; }
            if (m_stunTimer > 0.f) { m_stunTimer -= dt; return; }
            if (p1Alive && !m_target->IsInvincible())
            {
                const float dx = GetPosX() - m_target->GetPosX();
                const float dy = GetPosY() - m_target->GetPosY();
                if (dx * dx + dy * dy < HIT_RADIUS_SQ)
                    m_target->Kill();
            }
            break;

        // ── Dead (burger-crushed) ────────────────────────────────────────
        case VState::Dead:
            if (p1Dead) { m_state = VState::LockedP1Dead; m_respawnTimer = 0.f; return; }
            if (m_self) m_self->FreezeFor(1.f);
            m_respawnTimer -= dt;
            if (m_respawnTimer <= 0.f) StartEntering();
            break;

        // ── Waiting for P1 to respawn ────────────────────────────────────
        case VState::LockedP1Dead:
            if (p1Alive) StartEntering();
            else if (m_self) m_self->FreezeFor(1.f);
            break;
        }
    }

    void Render() override {}

    // ── Burger-carry interface (mirrors EnemyComponent) ──────────────────
    void CatchByBurger()
    {
        if (m_state != VState::Active) return;
        m_caughtByBurger = true;
        if (m_self) m_self->FreezeFor(1.f);
    }

    void SetFallingY(float y)
    {
        if (!m_caughtByBurger) return;
        if (m_self) m_self->SetPosition(m_self->GetPosX(), y);
    }

    void RecoverFromBurger(float landingY)
    {
        m_caughtByBurger = false;
        if (m_self)
        {
            m_self->SetPosition(m_self->GetPosX(), landingY);
            m_self->FreezeFor(0.f);
        }
    }

    bool IsCaughtByBurger() const { return m_caughtByBurger; }

    void Stun(float duration)
    {
        if (m_state != VState::Active) return;
        m_stunTimer = duration;
        if (m_self) m_self->FreezeFor(duration);
    }

    void Kill()
    {
        if (m_state != VState::Active) return;
        m_stunTimer    = 0.f;
        m_respawnTimer = RESPAWN_DELAY;
        m_state        = VState::Dead;
        if (m_self) m_self->SetPosition(-2000.f, -2000.f);
    }

    // Called when P2 was carried into a cup by a burger.
    // Score is awarded by BurgerPieceComponent before calling this.
    void LandedInCup()
    {
        m_caughtByBurger = false;
        m_stunTimer      = 0.f;

        // Snap to the nearest ladder X so we can climb back up (mirrors AI enemy behavior)
        float currentX = GetPosX();
        float currentY = GetPosY();
        if (m_levelMap)
        {
            const auto& ladders = m_levelMap->GetLadders();
            float bestDist = 1e9f;
            float bestX    = currentX;
            for (const auto& l : ladders)
            {
                float d = std::abs(l.x - currentX);
                if (d < bestDist) { bestDist = d; bestX = l.x; }
            }
            if (m_self) m_self->SetPosition(bestX, currentY);
        }
        m_state = VState::ClimbingFromCup;
    }

    bool IsAlive()    const { return m_state == VState::Active
                                  || m_state == VState::WalkingIn
                                  || m_state == VState::Entering
                                  || m_state == VState::ClimbingFromCup; }
    bool IsStunned()  const { return m_state == VState::Active && m_stunTimer > 0.f; }
    bool IsDying()    const { return false; } // no death anim on VS enemy for now
    bool IsEntering()        const { return m_state == VState::Entering || m_state == VState::WalkingIn; }
    bool IsEnteringRight()   const { return m_enterDir > 0.f; }
    bool IsClimbingFromCup() const { return m_state == VState::ClimbingFromCup; }

    float GetPosX() const { return m_self ? m_self->GetPosX() : 0.f; }
    float GetPosY() const { return m_self ? m_self->GetPosY() : 0.f; }

private:
    enum class VState { Entering, WalkingIn, Active, ClimbingFromCup, Dead, LockedP1Dead };

    PlatformMovementComponent* m_self;
    PlatformMovementComponent* m_target;
    const dae::LevelMap*       m_levelMap;
    glm::vec2 m_entryPos;
    float     m_enterDir   { 1.f };
    float     m_entryX     { 0.f };  // set from entryPos.x in constructor, walks from there
    float     m_entrySpeed { 40.f };
    float     m_platSnap   { 8.f };

    bool   m_caughtByBurger{ false };
    VState m_state         { VState::Entering };
    float  m_stunTimer   { 0.f };
    float  m_respawnTimer{ 0.f };
    float  m_lockTimer   { 0.f };

    static constexpr float HIT_RADIUS_SQ = 12.f * 12.f;
    static constexpr float RESPAWN_DELAY = 4.f;
    static constexpr float LOCK_ON_LAND  = 0.5f;   // brief freeze when landing on platform

    void HideAndWait()
    {
        m_state = VState::LockedP1Dead;
        if (m_self) m_self->SetPosition(-2000.f, -2000.f);
    }

    void StartEntering()
    {
        m_entryX   = m_entryPos.x;
        m_stunTimer = 0.f;
        if (m_target)
            m_enterDir = (m_target->GetPosX() >= m_entryPos.x) ? 1.f : -1.f;
        if (m_self) m_self->SetPosition(m_entryPos.x, m_entryPos.y);
        m_state = VState::Entering;
    }
};
