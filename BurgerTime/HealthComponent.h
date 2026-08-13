#pragma once
#include "BaseComponent.h"
#include "Subject.h"

namespace dae
{
    class GameObject;

    class HealthComponent : public BaseComponent
    {
    public:
        HealthComponent(GameObject& pOwner, int lives) : BaseComponent(pOwner), m_MaxLives{ lives }, m_Lives{ lives } {}
        ~HealthComponent() = default;
        HealthComponent(const HealthComponent&)            = delete;
        HealthComponent(HealthComponent&&)                 = delete;
        HealthComponent& operator=(const HealthComponent&) = delete;
        HealthComponent& operator=(HealthComponent&&)      = delete;

        void Update() override {}
        void Render() override {}

        // Lose one life and die. Fires LifeChanged with remaining lives.
        void LoseLife();

        // Gain one extra life (1UP). Fires LifeChanged with new count.
        void GainLife();

        int  GetLives()    const { return m_Lives; }
        int  GetMaxLives() const { return m_MaxLives; }
        bool IsAlive()     const { return m_Lives > 0; }

        Subject& GetSubject() { return m_Subject; }

    private:
        int     m_MaxLives;
        int     m_Lives;
        Subject m_Subject;
    };
}
