#pragma once
#include "BaseComponent.h"
#include "AnimatedSpriteComponent.h"
#include "EnemyComponent.h"
#include "PlatformMovementComponent.h"
#include "VersusEnemyPlayerComponent.h"
#include "TimeSingleton.h"

// ---------------------------------------------------------------------------
// EnemyAnimatorComponent
// Queries EnemyComponent state each frame and drives AnimatedSpriteComponent.
// ---------------------------------------------------------------------------
class EnemyAnimatorComponent : public dae::BaseComponent
{
public:
    EnemyAnimatorComponent(dae::GameObject& owner,
                           EnemyComponent* enemy,
                           AnimatedSpriteComponent* anim)
        : BaseComponent(owner), m_enemy{ enemy }, m_anim{ anim }
    {}

    // Register the correct clip set for the given enemy type.
    // Called once after the AnimatedSpriteComponent is created.
    static void RegisterClips(AnimatedSpriteComponent* anim, EnemyType type)
    {
        switch (type)
        {
        case EnemyType::Egg:
            anim->AddClip("walk_d",  "bt_egg_walk_d.png",   2, 6.f);
            anim->AddClip("walk_h",  "bt_egg_walk_h.png",   2, 6.f);
            anim->AddClip("walk_u",  "bt_egg_walk_u.png",   2, 6.f);
            anim->AddClip("squish",  "bt_egg_squish.png",   4, 6.f, false);
            anim->AddClip("stunned", "bt_egg_stunned.png",  2, 4.f);
            break;
        case EnemyType::Pickle:
            anim->AddClip("walk_d",  "bt_pickle_walk_d.png",   2, 6.f);
            anim->AddClip("walk_h",  "bt_pickle_walk_h.png",   2, 6.f);
            anim->AddClip("walk_u",  "bt_pickle_walk_u.png",   2, 6.f);
            anim->AddClip("squish",  "bt_pickle_squish.png",   4, 6.f, false);
            anim->AddClip("stunned", "bt_pickle_stunned.png",  2, 4.f);
            break;
        default: // Hotdog
            anim->AddClip("walk_d",  "bt_hotdog_walk_d.png",   2, 6.f);
            anim->AddClip("walk_h",  "bt_hotdog_walk_h.png",   2, 6.f);
            anim->AddClip("walk_u",  "bt_hotdog_walk_u.png",   2, 6.f);
            anim->AddClip("squish",  "bt_hotdog_squish.png",   4, 6.f, false);
            anim->AddClip("stunned", "bt_hotdog_stunned.png",  2, 4.f);
            break;
        }
    }

    void Update() override
    {
        if (!m_enemy || !m_anim) return;

        if (m_enemy->IsDeadAnim())
            return;

        // Level-clear freeze: stop animation in place
        if (m_enemy->IsLevelClearFrozen())
        {
            m_anim->Pause();
            return;
        }

        // Carried by falling burger, freeze on last frame
        if (m_enemy->IsFallingWithBurger())
        {
            m_anim->Pause();
            return;
        }

        // Squish: burger landed on top of enemy
        if (m_enemy->IsSquishedAnim())
        {
            m_anim->SetFlipH(false);
            m_anim->Play("squish");
            return;
        }

        // Pepper stunned / carried by burger / recovering
        if (m_enemy->IsStunnedAnim())
        {
            m_anim->SetFlipH(false);
            m_anim->Play("stunned");
            return;
        }

        // Walk — choose clip based on movement direction
        // Convention: dir.y > 0 = moving up (y decreasing), dir.y < 0 = moving down
        //             dir.x != 0 = horizontal; base sprite faces LEFT, flip for RIGHT
        glm::vec2 dir = m_enemy->GetMovementDir();
        if (dir.x != 0.f)
        {
            m_anim->SetFlipH(dir.x > 0.f); // base=LEFT, flip for RIGHT
            m_anim->Play("walk_h");
        }
        else if (dir.y > 0.f)
        {
            m_anim->SetFlipH(false);
            m_anim->Play("walk_u");
        }
        else
        {
            m_anim->SetFlipH(false);
            m_anim->Play("walk_d");
        }
    }

    void Render() override {}

private:
    EnemyComponent*         m_enemy;
    AnimatedSpriteComponent* m_anim;
};

// ---------------------------------------------------------------------------
// PlayerAnimatorComponent
// Priority order: dying > peppering > idle > walking
// ---------------------------------------------------------------------------
class PlayerAnimatorComponent : public dae::BaseComponent
{
public:
    PlayerAnimatorComponent(dae::GameObject& owner,
                            PlatformMovementComponent* player,
                            AnimatedSpriteComponent*   anim)
        : BaseComponent(owner), m_player{ player }, m_anim{ anim }
    {}

    // Called by PepperComponent callback when pepper is fired
    void OnPepperFired()
    {
        m_pepperTimer = PEPPER_ANIM_DURATION;
        m_pepperDir   = m_player->GetFacingDir();
    }

    // Called when round is cleared, locks the celebrate clip until next level
    void SetCelebrating(bool on) { m_celebrating = on; }

    void Update() override
    {
        if (!m_player || !m_anim) return;

        const float dt = dae::Time::GetInstance().GetDeltaTime();

        // ── Round-clear celebration ──────────────────────────────────────
        if (m_celebrating)
        {
            m_anim->SetFlipH(false);
            m_anim->Play("celebrate");
            return;
        }

        // ── Death animation ──────────────────────────────────────────────
        if (m_player->IsDying())
        {
            m_anim->SetFlipH(false);
            m_anim->Play("die");
            return;
        }

        // Player is fully dead (off-screen) nothing to render
        if (!m_player->IsAlive()) return;

        // Pepper throw pose (movement is already frozen in PlatformMovementComponent)
        if (m_pepperTimer > 0.f)
        {
            m_pepperTimer -= dt;
            if (m_pepperDir.x != 0.f)
            {
                m_anim->SetFlipH(m_pepperDir.x > 0.f);
                m_anim->Play("pepper_h");
            }
            else if (m_pepperDir.y > 0.f)
            {
                m_anim->SetFlipH(false);
                m_anim->Play("pepper_u");
            }
            else
            {
                m_anim->SetFlipH(false);
                m_anim->Play("pepper_d");
            }
            return;
        }

        // ── Idle ─────────────────────────────────────────────────────────
        if (!m_player->IsMoving())
        {
            m_anim->SetFlipH(false);
            // Use back-turned idle if the player last moved upward
            m_anim->Play(m_lastDir.y > 0.f ? "idle_u" : "idle");
            return;
        }

        // ── Walk ─────────────────────────────────────────────────────────
        glm::vec2 dir = m_player->GetFacingDir();
        m_lastDir = dir; // remember for idle pose
        if (dir.x != 0.f)
        {
            m_anim->SetFlipH(dir.x > 0.f); // base=LEFT, flip for RIGHT
            m_anim->Play("walk_h");
        }
        else if (dir.y > 0.f)
        {
            m_anim->SetFlipH(false);
            m_anim->Play("walk_u");
        }
        else
        {
            m_anim->SetFlipH(false);
            m_anim->Play("walk_d");
        }
    }

    void Render() override {}

private:
    PlatformMovementComponent* m_player;
    AnimatedSpriteComponent*   m_anim;
    glm::vec2 m_lastDir{ 0.f, -1.f };   // last movement direction, for idle pose selection
    float     m_pepperTimer{ 0.f };
    glm::vec2 m_pepperDir{ 0.f, -1.f }; // facing dir captured at throw moment
    bool      m_celebrating{ false };

    static constexpr float PEPPER_ANIM_DURATION = 0.5f;
};

// ---------------------------------------------------------------------------
// VersusEnemyAnimatorComponent
// Drives enemy-style clips (walk_h / walk_u / walk_d / idle) from a
// PlatformMovementComponent.  Used for the player-controlled hot dog in
// Versus mode.  Clips follow the same naming / flip convention as enemies.
// ---------------------------------------------------------------------------
class VersusEnemyAnimatorComponent : public dae::BaseComponent
{
public:
    VersusEnemyAnimatorComponent(dae::GameObject& owner,
                                 PlatformMovementComponent*    player,
                                 AnimatedSpriteComponent*      anim,
                                 VersusEnemyPlayerComponent*   versusEnemy = nullptr)
        : BaseComponent(owner), m_player{ player }, m_anim{ anim }, m_versusEnemy{ versusEnemy }
    {}

    void Update() override
    {
        if (!m_player || !m_anim) return;

        // Dead / locked — P2 is off-screen, nothing to render
        if (m_versusEnemy && !m_versusEnemy->IsAlive()) return;

        // Entering: P2 is auto-walking horizontally onto the level
        if (m_versusEnemy && m_versusEnemy->IsEntering())
        {
            // WalkingIn sub-state = just landed, brief idle before control
            // Entering sub-state = actually moving, play walk_h
            m_anim->SetFlipH(m_versusEnemy->IsEnteringRight()); // base sprite faces LEFT
            m_anim->Play("walk_h");
            return;
        }

        // Climbing out of cup — play walk_u animation
        if (m_versusEnemy && m_versusEnemy->IsClimbingFromCup())
        {
            m_anim->SetFlipH(false);
            m_anim->Play("walk_u");
            return;
        }

        // Squish death animation
        if (m_versusEnemy && m_versusEnemy->IsDying())
        {
            m_anim->SetFlipH(false);
            m_anim->Play("squish");
            return;
        }

        // Stunned
        if (m_versusEnemy && m_versusEnemy->IsStunned())
        {
            m_anim->SetFlipH(false);
            m_anim->Play("stunned");
            return;
        }

        // No separate idle — play walk clip for current facing dir (same as AI enemies)
        glm::vec2 dir = m_player->GetFacingDir();
        if (dir.x != 0.f)
        {
            m_anim->SetFlipH(dir.x > 0.f); // base sprite faces LEFT
            m_anim->Play("walk_h");
        }
        else if (dir.y > 0.f)
        {
            m_anim->SetFlipH(false);
            m_anim->Play("walk_u");
        }
        else
        {
            m_anim->SetFlipH(false);
            m_anim->Play("walk_d");
        }
    }

    void Render() override {}

private:
    PlatformMovementComponent*  m_player;
    AnimatedSpriteComponent*    m_anim;
    VersusEnemyPlayerComponent* m_versusEnemy;
};
