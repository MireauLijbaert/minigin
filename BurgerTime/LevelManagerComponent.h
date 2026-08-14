#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "Event.h"
#include "BurgerPieceComponent.h"
#include "TimeSingleton.h"
#include "GameObject.h"
#include "InputManager.h"
#include "Command.h"
#include <functional>
#include <vector>
#include <SDL3/SDL.h>

class LevelManagerComponent : public dae::BaseComponent
{
public:
    static constexpr float COMPLETE_DELAY = 3.5f;

    dae::Subject& GetSubject() { return m_subject; }
    LevelManagerComponent(dae::GameObject& owner,
                          std::vector<BurgerPieceComponent*>* burgers,
                          std::function<void()> onComplete)
        : BaseComponent(owner)
        , m_burgers{ burgers }
        , m_onComplete{ std::move(onComplete) }
    {
        // F1 skip: bind on key-UP so holding does nothing
        dae::InputManager::GetInstance().BindKeyboardInput(
            SDL_SCANCODE_F1,
            std::make_unique<dae::LambdaCommand>([this]()
            {
                if (!m_triggered && !m_completing)
                {
                    m_completing = true;
                    m_timer = 0.f;
                }
            }),
            dae::InputState::Up
        );
    }

    // Overwrite the F1 binding with a no-op on destruction so the InputManager
    // never holds a dangling 'this' capture after the scene changes.
    ~LevelManagerComponent() override
    {
        dae::InputManager::GetInstance().BindKeyboardInput(
            SDL_SCANCODE_F1,
            std::make_unique<dae::LambdaCommand>([](){}),
            dae::InputState::Up);
    }

    void Render() override {}

    void Update() override
    {
        if (m_triggered) return;

        if (!m_completing)
        {
            if (!m_burgers || m_burgers->empty()) return;
            bool allDone = true;
            for (auto* b : *m_burgers)
                if (!b->IsInCup()) { allDone = false; break; }
            if (allDone)
            {
                m_completing = true;
                m_subject.NotifyObservers(dae::Event("LevelComplete"), GetOwner());
            }
        }

        if (!m_completing) return;

        m_timer -= dae::Time::GetInstance().GetDeltaTime();
        if (m_timer <= 0.f)
        {
            m_triggered = true;
            m_onComplete();
        }
    }

private:
    std::vector<BurgerPieceComponent*>* m_burgers;
    std::function<void()> m_onComplete;
    bool m_triggered{ false };
    bool m_completing{ false };
    float m_timer{ COMPLETE_DELAY };

    dae::Subject m_subject;
};
