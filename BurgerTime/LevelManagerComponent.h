#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "Event.h"
#include "BurgerPieceComponent.h"
#include "TimeSingleton.h"
#include "GameObject.h"
#include <functional>
#include <vector>
#include <SDL3/SDL.h>

class LevelManagerComponent : public dae::BaseComponent
{
public:
    dae::Subject& GetSubject() { return m_subject; }
    LevelManagerComponent(dae::GameObject& owner,
                          std::vector<BurgerPieceComponent*>* burgers,
                          std::function<void()> onComplete)
        : BaseComponent(owner)
        , m_burgers{ burgers }
        , m_onComplete{ std::move(onComplete) }
    {}

    void Render() override {}

    void Update() override
    {
        if (m_triggered) return;

        const auto* keys = SDL_GetKeyboardState(nullptr);
        bool skipPressed = keys[SDL_SCANCODE_N] != 0;
        if (skipPressed && !m_prevSkip)
        {
            m_completing = true;
            m_timer = 0.f;
        }
        m_prevSkip = skipPressed;

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
    bool m_prevSkip{ false };
    float m_timer{ COMPLETE_DELAY };

    dae::Subject m_subject;
    static constexpr float COMPLETE_DELAY = 3.5f;
};
