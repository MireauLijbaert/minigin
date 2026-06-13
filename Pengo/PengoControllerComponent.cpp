#include "PengoControllerComponent.h"
#include "PengoStates.h"
#include "GridMovementComponent.h"
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
            input.BindKeyboardInput(SDL_SCANCODE_W,     std::make_unique<GridMoveCommand>(owner, glm::ivec2{  0, -1 }), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_A,     std::make_unique<GridMoveCommand>(owner, glm::ivec2{ -1,  0 }), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_S,     std::make_unique<GridMoveCommand>(owner, glm::ivec2{  0,  1 }), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_D,     std::make_unique<GridMoveCommand>(owner, glm::ivec2{  1,  0 }), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_SPACE, std::make_unique<PushCommand>(owner),                           InputState::Down);
        }
        else if (m_KeyboardScheme == KeyboardScheme::IJKL)
        {
            input.BindKeyboardInput(SDL_SCANCODE_I,      std::make_unique<GridMoveCommand>(owner, glm::ivec2{  0, -1 }), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_J,      std::make_unique<GridMoveCommand>(owner, glm::ivec2{ -1,  0 }), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_K,      std::make_unique<GridMoveCommand>(owner, glm::ivec2{  0,  1 }), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_L,      std::make_unique<GridMoveCommand>(owner, glm::ivec2{  1,  0 }), InputState::Held);
            input.BindKeyboardInput(SDL_SCANCODE_RETURN, std::make_unique<PushCommand>(owner),                           InputState::Down);
        }
        // KeyboardScheme::None: no keyboard bindings

        input.BindGamepadInput(GamepadButton::DPadUp,    std::make_unique<GridMoveCommand>(owner, glm::ivec2{  0, -1 }), InputState::Held, m_GamepadIndex);
        input.BindGamepadInput(GamepadButton::DPadDown,  std::make_unique<GridMoveCommand>(owner, glm::ivec2{  0,  1 }), InputState::Held, m_GamepadIndex);
        input.BindGamepadInput(GamepadButton::DPadLeft,  std::make_unique<GridMoveCommand>(owner, glm::ivec2{ -1,  0 }), InputState::Held, m_GamepadIndex);
        input.BindGamepadInput(GamepadButton::DPadRight, std::make_unique<GridMoveCommand>(owner, glm::ivec2{  1,  0 }), InputState::Held, m_GamepadIndex);
        input.BindGamepadInput(GamepadButton::A,            std::make_unique<PushCommand>(owner),                          InputState::Down, m_GamepadIndex);

        m_StateMachine.SetState(std::make_unique<PengoIdleState>(*this));
    }

    void PengoControllerComponent::Update()
    {
        if (!m_StateMachine.IsInState<PengoDyingState>())
        {
            auto* mov = GetOwner()->GetComponent<GridMovementComponent>();
            const bool isMoving = mov && mov->IsMoving();

            if (isMoving)
                m_StateMachine.SetState(std::make_unique<PengoMovingState>(*this));
            else
                m_StateMachine.SetState(std::make_unique<PengoIdleState>(*this));
        }

        m_StateMachine.Update();
    }

    void PengoControllerComponent::Die()
    {
        m_StateMachine.SetState(std::make_unique<PengoDyingState>(*this));
    }

    void PengoControllerComponent::Respawn(glm::ivec2 spawnCell)
    {
        m_StateMachine.SetState(std::make_unique<PengoIdleState>(*this));
        if (auto* mov = GetOwner()->GetComponent<GridMovementComponent>())
            mov->WarpTo(spawnCell);
    }
}