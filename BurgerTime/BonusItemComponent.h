#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "Event.h"
#include "PlatformMovementComponent.h"
#include "PepperComponent.h"
#include "ScoreManager.h"
#include "TimeSingleton.h"
#include "RenderComponent.h"
#include "GameObject.h"
#include <glm/glm.hpp>

// Bonus item that appears every CUPS_PER_BONUS burger pieces landing in cups.
// Gives score + 1 pepper charge on pickup.
class BonusItemComponent : public dae::BaseComponent
{
public:
    dae::Subject& GetSubject() { return m_subject; }

    BonusItemComponent(dae::GameObject& owner,
                       glm::vec2 worldPos,        // center of the item (world coords)
                       float size,                 // width = height (match charW)
                       PlatformMovementComponent* player,
                       PepperComponent* pepper,
                       dae::RenderComponent* renderComp,
                       int score = 500,
                       float activeTime = 6.f)
        : BaseComponent(owner)
        , m_pos{ worldPos }
        , m_halfSize{ size * 0.5f }
        , m_player{ player }
        , m_pepper{ pepper }
        , m_renderComp{ renderComp }
        , m_score{ score }
        , m_activeTime{ activeTime }
    {
        if (m_renderComp)
            m_renderComp->SetVisible(false);
    }

    // Called by each BurgerPieceComponent when it lands in a cup.
    // Every CUPS_PER_BONUS calls triggers a bonus appearance.
    void OnBurgerInCup()
    {
        ++m_cupCount;
        if (m_cupCount % CUPS_PER_BONUS == 0 && !m_active)
        {
            m_active = true;
            m_timer  = m_activeTime;
            if (m_renderComp) m_renderComp->SetVisible(true);
            m_subject.NotifyObservers(dae::Event("BonusAppeared"), GetOwner());
        }
    }

    void Update() override
    {
        if (!m_active) return;

        const float dt = dae::Time::GetInstance().GetDeltaTime();
        m_timer -= dt;

        if (m_timer <= 0.f)
        {
            m_active = false;
            if (m_renderComp) m_renderComp->SetVisible(false);
            return;
        }

        // Player overlap check
        if (m_player->IsAlive())
        {
            float dx        = m_player->GetPosX() - m_pos.x;
            float dy        = m_player->GetPosY() - m_pos.y;
            float threshold = m_halfSize * 1.6f;
            if (dx * dx + dy * dy < threshold * threshold)
                Pickup();
        }
    }

    void Render() override {} // RenderComponent handles drawing

private:
    glm::vec2 m_pos;
    float     m_halfSize;
    PlatformMovementComponent* m_player;
    PepperComponent*           m_pepper;
    dae::RenderComponent*      m_renderComp;
    int   m_score;
    float m_activeTime;
    float m_timer { 0.f };
    bool  m_active{ false };
    int   m_cupCount{ 0 };

    static constexpr int CUPS_PER_BONUS = 3;

    dae::Subject m_subject;

    void Pickup()
    {
        m_active = false;
        m_timer  = 0.f;
        if (m_renderComp) m_renderComp->SetVisible(false);
        ScoreManager::GetInstance().AddScore(m_score);
        m_pepper->AddCharge(1);
        m_subject.NotifyObservers(dae::Event("BonusPickedUp"), GetOwner());
    }
};
