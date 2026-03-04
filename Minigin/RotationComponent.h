// Temporary component for this week, for testing purposes in hand in
#pragma once
#include "BaseComponent.h"
#include "GameObject.h"
#include <cmath>

namespace dae
{
    class RotationComponent : public BaseComponent
    {
    public:
        RotationComponent(GameObject& owner, float angularSpeed, float radius, float initialAngle = 0.f)
            : BaseComponent(owner), m_speed(angularSpeed), m_radius(radius), m_angle(initialAngle) {
        }

        void Update() override
        {
            m_angle += m_speed;
            if (m_angle >= 360.f)
                m_angle -= 360.f;
			else if (m_angle < 0.f)
                m_angle += 360.f;

            const float rad = m_angle * 3.14159265f / 180.f;
            const float x = m_radius * std::cos(rad);
            const float y = m_radius * std::sin(rad);
            GetOwner()->SetLocalPosition(x, y);
        }

        void Render() override {}

    private:
        float m_speed;  // degrees per update
        float m_radius; // distance from parent
        float m_angle;  // current rotation angle
    };
}