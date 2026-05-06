#pragma once
#include "BaseComponent.h"
#include "StateMachine.h"
#include <glm/glm.hpp>

namespace dae
{
    class GameObject;

    // Have set schemes as we only will have 2 players anyway and we dont really need the ability to choose buttons
    enum class KeyboardScheme { WASD, IJKL, None };

    class PengoControllerComponent : public BaseComponent
    {
        StateMachine m_StateMachine;
        float m_MoveSpeed{ 100.f };
        int m_GamepadIndex{ 1 };
        KeyboardScheme m_KeyboardScheme{ KeyboardScheme::WASD };

    public:
        explicit PengoControllerComponent(GameObject& owner, float moveSpeed = 100.f, int gamepadIndex = 1, KeyboardScheme keyboard = KeyboardScheme::WASD);

        void Update(float deltaTime);
        void Render() const {}

        float GetMoveSpeed() const { return m_MoveSpeed; }

        void Die();
    };
}