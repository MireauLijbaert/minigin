#pragma once
#include "BaseComponent.h"
#include <memory>
#include <glm/glm.hpp>
#include "GameObject.h"

namespace dae
{
	class GameObject;

	class MovementComponent : public BaseComponent
	{
	public:

		MovementComponent(GameObject& pOwner, float maxSpeed) : BaseComponent(pOwner), m_MaxSpeed(maxSpeed) {}
		~MovementComponent() = default;
		MovementComponent(const MovementComponent& other) = default;
		MovementComponent(MovementComponent&& other) = default;
		MovementComponent& operator=(const MovementComponent& other) = default;
		MovementComponent& operator=(MovementComponent&& other) = default;

		void Update() override {};
		void Render() override {};

		void Move(glm::vec3& direction);
		void Move(float x, float y, float z = 0.f);

	private:
		float m_MaxSpeed{};
	};
}