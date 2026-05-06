#include "PengoControllerComponent.h"
#include "PengoStates.h"
#include "InputManager.h"
#include "Commands.h"
#include "GameObject.h"
#include <SDL3/SDL.h>

namespace dae
{
    PengoControllerComponent::PengoControllerComponent(GameObject& owner, float moveSpeed, int gamepadIndex, KeyboardScheme keyboard)
        : BaseComponent(owner)
        , m_MoveSpeed(moveSpeed)
        , m_GamepadIndex(gamepadIndex)
        , m_KeyboardScheme(keyboard)
    {
        auto& input = InputManager::GetInstance();

        if (m_KeyboardScheme == KeyboardScheme::WASD)
        {
            input.BindKeyboardInput(SDL_SCANCODE_W, std::make_unique<MovementCommand>(owner, glm::vec3{ 0.f, -1.f, 0.f }, m_MoveSpeed), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_A, std::make_unique<MovementCommand>(owner, glm::vec3{ -1.f, 0.f, 0.f }, m_MoveSpeed), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_S, std::make_unique<MovementCommand>(owner, glm::vec3{ 0.f,  1.f, 0.f }, m_MoveSpeed), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_D, std::make_unique<MovementCommand>(owner, glm::vec3{ 1.f,  0.f, 0.f }, m_MoveSpeed), InputState::Held);
        }
        else if (m_KeyboardScheme == KeyboardScheme::IJKL)
        {
            input.BindKeyboardInput(SDL_SCANCODE_I, std::make_unique<MovementCommand>(owner, glm::vec3{ 0.f, -1.f, 0.f }, m_MoveSpeed), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_J, std::make_unique<MovementCommand>(owner, glm::vec3{ -1.f, 0.f, 0.f }, m_MoveSpeed), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_K, std::make_unique<MovementCommand>(owner, glm::vec3{ 0.f,  1.f, 0.f }, m_MoveSpeed), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_L, std::make_unique<MovementCommand>(owner, glm::vec3{ 1.f,  0.f, 0.f }, m_MoveSpeed), InputState::Held);
        }
        // KeyboardScheme::None — no keyboard bindings

        input.BindGamepadInput(GamepadButton::DPadUp, std::make_unique<MovementCommand>(owner, glm::vec3{ 0.f, -1.f, 0.f }, m_MoveSpeed), InputState::Held, m_GamepadIndex);
        input.BindGamepadInput(GamepadButton::DPadDown, std::make_unique<MovementCommand>(owner, glm::vec3{ 0.f,  1.f, 0.f }, m_MoveSpeed), InputState::Held, m_GamepadIndex);
        input.BindGamepadInput(GamepadButton::DPadLeft, std::make_unique<MovementCommand>(owner, glm::vec3{ -1.f, 0.f, 0.f }, m_MoveSpeed), InputState::Held, m_GamepadIndex);
        input.BindGamepadInput(GamepadButton::DPadRight, std::make_unique<MovementCommand>(owner, glm::vec3{ 1.f,  0.f, 0.f }, m_MoveSpeed), InputState::Held, m_GamepadIndex);

        m_StateMachine.SetState(std::make_unique<PengoIdleState>(*this));
    }

    void PengoControllerComponent::Update(float deltaTime)
    {
        m_StateMachine.Update(deltaTime);
    }

    void PengoControllerComponent::Die()
    {
        m_StateMachine.SetState(std::make_unique<PengoDyingState>(*this));
    }
}