#pragma once
#include "BaseComponent.h"
#include "Observer.h"
#include "Subject.h"
#include "Event.h"
#include "HealthComponent.h"
#include <functional>

class GameOverWatcherComponent : public dae::BaseComponent, public dae::Observer
{
public:
    GameOverWatcherComponent(dae::GameObject& owner,
                             dae::HealthComponent* health,
                             std::function<void()> onGameOver)
        : BaseComponent(owner)
        , m_subject{ &health->GetSubject() }
        , m_onGameOver{ std::move(onGameOver) }
    {
        m_subject->AddObserver(this);
    }

    ~GameOverWatcherComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* subject) override
    {
        if (subject == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (m_triggered) return;
        if (event.id == "LifeChanged" && event.args[0].intValue == 0)
        {
            m_triggered = true;
            m_onGameOver();
        }
    }

    void Update() override {}
    void Render() override {}

private:
    dae::Subject* m_subject;
    std::function<void()> m_onGameOver;
    bool m_triggered{ false };
};
