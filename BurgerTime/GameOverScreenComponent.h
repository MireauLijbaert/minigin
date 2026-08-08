#pragma once
#include "BaseComponent.h"
#include <functional>
#include <SDL3/SDL.h>

class GameOverScreenComponent : public dae::BaseComponent
{
public:
    GameOverScreenComponent(dae::GameObject& owner, std::function<void()> onRestart)
        : BaseComponent(owner)
        , m_onRestart{ std::move(onRestart) }
    {}

    void Render() override {}

    void Update() override
    {
        const auto* keys = SDL_GetKeyboardState(nullptr);
        bool pressed = keys[SDL_SCANCODE_RETURN] != 0 || keys[SDL_SCANCODE_R] != 0;
        if (pressed && !m_prevKey)
        {
            m_prevKey = true;
            m_onRestart();
        }
        m_prevKey = pressed;
    }

private:
    std::function<void()> m_onRestart;
    bool m_prevKey{ false };
};
