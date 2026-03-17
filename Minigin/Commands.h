#pragma once
#include "GameObject.h"

namespace dae
{
	class Command
	{
	public:
		virtual ~Command() = default;
		virtual void Execute() = 0;
	};

	class MovementCommand : public Command
	{
	public:
		MovementCommand(GameObject& actor, const glm::vec3& direction, const float speed) : m_actor(&actor), m_direction(direction), m_speed(speed) {}
		void Execute() override;
	private:
		GameObject* m_actor;
		glm::vec3 m_direction;
		float m_speed;
	};

}