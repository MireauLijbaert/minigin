#pragma once
#include "BaseComponent.h"
#include "HealthComponent.h"
#include <functional>

class GameOverWatcherComponent : public dae::BaseComponent
{
public:
    GameOverWatcherComponent(dae::GameObject& owner,
                             dae::HealthComponent* health,
                             std::function<void()> onGameOver)
        : BaseComponent(owner)
        , m_health{ health }
        , m_onGameOver{ std::move(onGameOver) }
    {}

    void Render() override {}

    void Update() override
    {
        if (m_triggered) return;
        if (m_health && !m_health->IsAlive())
        {
            m_triggered = true;
            m_onGameOver();
        }
    }

private:
    dae::HealthComponent* m_health;
    std::function<void()> m_onGameOver;
    bool m_triggered{ false };
};
