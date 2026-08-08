#pragma once
#include "BaseComponent.h"
#include "TextComponent.h"
#include "HealthComponent.h"
#include "PepperComponent.h"

// ---- Score singleton --------------------------------------------------------

class ScoreManager
{
public:
    static ScoreManager& GetInstance()
    {
        static ScoreManager instance;
        return instance;
    }

    void AddScore(int amount) { m_score += amount; }
    int  GetScore()  const   { return m_score; }
    void Reset()             { m_score = 0; }

private:
    ScoreManager() = default;
    int m_score{ 0 };
};

// ---- HUD display components (header-only) -----------------------------------

class ScoreDisplayComponent : public dae::BaseComponent
{
public:
    ScoreDisplayComponent(dae::GameObject& owner, dae::TextComponent* text)
        : BaseComponent(owner), m_text{ text } {}

    void Update() override
    {
        int score = ScoreManager::GetInstance().GetScore();
        if (score != m_lastScore)
        {
            m_lastScore = score;
            m_text->SetText("Score: " + std::to_string(score));
        }
    }
    void Render() override {}

private:
    dae::TextComponent* m_text;
    int m_lastScore{ -1 };
};

class PepperDisplayComponent : public dae::BaseComponent
{
public:
    PepperDisplayComponent(dae::GameObject& owner, dae::TextComponent* text,
                           PepperComponent* pepper)
        : BaseComponent(owner), m_text{ text }, m_pepper{ pepper } {}

    void Update() override
    {
        int charges = m_pepper->GetCharges();
        if (charges != m_lastCharges)
        {
            m_lastCharges = charges;
            m_text->SetText("Pepper: " + std::to_string(charges));
        }
    }
    void Render() override {}

private:
    dae::TextComponent* m_text;
    PepperComponent* m_pepper;
    int m_lastCharges{ -1 };
};

class LivesDisplayComponent : public dae::BaseComponent
{
public:
    LivesDisplayComponent(dae::GameObject& owner, dae::TextComponent* text,
                          dae::HealthComponent* health)
        : BaseComponent(owner), m_text{ text }, m_health{ health } {}

    void Update() override
    {
        int lives = m_health->GetLives();
        if (lives != m_lastLives)
        {
            m_lastLives = lives;
            m_text->SetText("Lives: " + std::to_string(lives));
        }
    }
    void Render() override {}

private:
    dae::TextComponent* m_text;
    dae::HealthComponent* m_health;
    int m_lastLives{ -1 };
};
