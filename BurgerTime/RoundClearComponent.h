#pragma once
#include "BaseComponent.h"
#include "Observer.h"
#include "Subject.h"
#include "Event.h"
#include "TextComponent.h"
#include "LevelManagerComponent.h"
#include "EnemyComponent.h"
#include "PlatformMovementComponent.h"
#include "CharacterAnimators.h"
#include <vector>

// On "LevelComplete":
//   - Shows "ROUND CLEAR!" text overlay
//   - Freezes all enemies (stops movement + pauses their animation)
//   - Freezes player movement (FreezeFor matches LevelManagerComponent::COMPLETE_DELAY)
//   - Triggers the player celebrate animation (hands-up alternation)

class RoundClearComponent : public dae::BaseComponent, public dae::Observer
{
public:
    RoundClearComponent(dae::GameObject& owner,
                        LevelManagerComponent*          mgr,
                        dae::TextComponent*             text,
                        std::vector<EnemyComponent*>*   enemies,
                        PlatformMovementComponent*      playerMove,
                        PlayerAnimatorComponent*        playerAnim,
                        PlatformMovementComponent*      playerMove2 = nullptr,
                        PlayerAnimatorComponent*        playerAnim2 = nullptr)
        : BaseComponent(owner)
        , m_text{ text }
        , m_enemies{ enemies }
        , m_playerMove{ playerMove }
        , m_playerMove2{ playerMove2 }
        , m_playerAnim{ playerAnim }
        , m_playerAnim2{ playerAnim2 }
        , m_subject{ mgr ? &mgr->GetSubject() : nullptr }
    {
        if (m_subject) m_subject->AddObserver(this);
    }

    ~RoundClearComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* s) override
    {
        if (s == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (event.id != "LevelComplete") return;

        // Show overlay text
        if (m_text)
            m_text->SetText("ROUND CLEAR!");

        // Freeze every enemy in place
        if (m_enemies)
            for (auto* e : *m_enemies)
                if (e) e->Freeze();

        // Stop player movement for the duration of the delay
        // (matches LevelManagerComponent::COMPLETE_DELAY = 3.5f)
        if (m_playerMove)
            m_playerMove->FreezeFor(LevelManagerComponent::COMPLETE_DELAY);
        if (m_playerMove2)
            m_playerMove2->FreezeFor(LevelManagerComponent::COMPLETE_DELAY);

        // Switch player(s) to celebrate animation
        if (m_playerAnim)
            m_playerAnim->SetCelebrating(true);
        if (m_playerAnim2)
            m_playerAnim2->SetCelebrating(true);
    }

    void Update() override {}
    void Render() override {}

private:
    dae::TextComponent*             m_text;
    std::vector<EnemyComponent*>*   m_enemies;
    PlatformMovementComponent*      m_playerMove;
    PlatformMovementComponent*      m_playerMove2;
    PlayerAnimatorComponent*        m_playerAnim;
    PlayerAnimatorComponent*        m_playerAnim2;
    dae::Subject*                   m_subject;
};
