#pragma once
#include "BaseComponent.h"
#include "Observer.h"
#include "Subject.h"
#include "Event.h"
#include "TextComponent.h"
#include "HealthComponent.h"
#include "PepperComponent.h"
#include "GameObject.h"

// ---- Score singleton --------------------------------------------------------

class ScoreManager
{
public:
    static ScoreManager& GetInstance()
    {
        static ScoreManager instance;
        return instance;
    }

    void AddScore(int amount)
    {
        m_score += amount;
        NotifyScoreChanged();
    }

    int GetScore() const { return m_score; }

    void Reset()
    {
        m_score = 0;
        NotifyScoreChanged();
    }

    dae::Subject& GetSubject() { return m_subject; }

private:
    ScoreManager() = default;
    int m_score{ 0 };
    dae::Subject m_subject;

    void NotifyScoreChanged()
    {
        dae::Event event{ "ScoreChanged" };
        event.nbArgs = 1;
        event.args[0] = dae::EventArg{ .intValue = m_score };
        m_subject.NotifyObservers(event, nullptr);
    }
};

// ---- HUD display components (Observer-based) --------------------------------

class ScoreDisplayComponent : public dae::BaseComponent, public dae::Observer
{
public:
    ScoreDisplayComponent(dae::GameObject& owner, dae::TextComponent* text)
        : BaseComponent(owner), m_text{ text }, m_subject{ &ScoreManager::GetInstance().GetSubject() }
    {
        m_text->SetText("Score: " + std::to_string(ScoreManager::GetInstance().GetScore()));
        m_subject->AddObserver(this);
    }

    ~ScoreDisplayComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* subject) override
    {
        if (subject == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (event.id == "ScoreChanged")
            m_text->SetText("Score: " + std::to_string(event.args[0].intValue));
    }

    void Update() override {}
    void Render() override {}

private:
    dae::TextComponent* m_text;
    dae::Subject* m_subject;
};

class PepperDisplayComponent : public dae::BaseComponent, public dae::Observer
{
public:
    PepperDisplayComponent(dae::GameObject& owner, dae::TextComponent* text,
                           PepperComponent* pepper)
        : BaseComponent(owner), m_text{ text }, m_subject{ &pepper->GetSubject() }
    {
        m_text->SetText("Pepper: " + std::to_string(pepper->GetCharges()));
        m_subject->AddObserver(this);
    }

    ~PepperDisplayComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* subject) override
    {
        if (subject == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (event.id == "PepperChanged")
            m_text->SetText("Pepper: " + std::to_string(event.args[0].intValue));
    }

    void Update() override {}
    void Render() override {}

private:
    dae::TextComponent* m_text;
    dae::Subject* m_subject;
};

class LivesDisplayComponent : public dae::BaseComponent, public dae::Observer
{
public:
    LivesDisplayComponent(dae::GameObject& owner, dae::TextComponent* text,
                          dae::HealthComponent* health)
        : BaseComponent(owner), m_text{ text }, m_subject{ &health->GetSubject() }
    {
        m_text->SetText("Lives: " + std::to_string(health->GetLives()));
        m_subject->AddObserver(this);
    }

    ~LivesDisplayComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* subject) override
    {
        if (subject == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (event.id == "LifeChanged")
            m_text->SetText("Lives: " + std::to_string(event.args[0].intValue));
    }

    void Update() override {}
    void Render() override {}

private:
    dae::TextComponent* m_text;
    dae::Subject* m_subject;
};
