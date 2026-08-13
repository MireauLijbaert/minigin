#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "Event.h"
#include "PlatformMovementComponent.h"
#include "PepperComponent.h"
#include "ScoreManager.h"
#include "TimeSingleton.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "GameObject.h"
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>

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
                       int score             = 500,
                       float activeTime      = 10.f,
                       const std::string& textureName = "")
        : BaseComponent(owner)
        , m_pos{ worldPos }
        , m_halfSize{ size * 0.5f }
        , m_player{ player }
        , m_pepper{ pepper }
        , m_score{ score }
        , m_activeTime{ activeTime }
    {
        if (!textureName.empty())
            m_tex = dae::ResourceManager::GetInstance().LoadTexture(textureName);
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

    void Render() override
    {
        if (!m_active) return;

        SDL_Renderer* r = dae::Renderer::GetInstance().GetSDLRenderer();
        SDL_FRect rect{
            m_pos.x - m_halfSize,
            m_pos.y - m_halfSize * 2.f,  // anchor bottom-center like characters
            m_halfSize * 2.f,
            m_halfSize * 2.f
        };

        if (m_tex)
        {
            SDL_RenderTexture(r, m_tex->GetSDLTexture(), nullptr, &rect);
        }
        else
        {
            // Fallback: gold rectangle
            SDL_SetRenderDrawColor(r, 255, 215, 0, 255);
            SDL_RenderFillRect(r, &rect);
            SDL_SetRenderDrawColor(r, 200, 100, 0, 255);
            SDL_RenderRect(r, &rect);
        }
    }

private:
    std::shared_ptr<dae::Texture2D> m_tex;
    glm::vec2 m_pos;
    float     m_halfSize;
    PlatformMovementComponent* m_player;
    PepperComponent*           m_pepper;
    int   m_score;
    float m_activeTime;
    float m_timer{ 0.f };
    bool  m_active{ false };
    int   m_cupCount{ 0 };

    static constexpr int CUPS_PER_BONUS = 3;

    dae::Subject m_subject;

    void Pickup()
    {
        m_active = false;
        m_timer  = 0.f;
        ScoreManager::GetInstance().AddScore(m_score);
        m_pepper->AddCharge(1);
        m_subject.NotifyObservers(dae::Event("BonusPickedUp"), GetOwner());
    }
};
