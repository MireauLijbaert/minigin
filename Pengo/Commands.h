#pragma once
#include "Command.h"
#include "GameObject.h"
#include <glm/glm.hpp>

namespace dae
{

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

	class GridMoveCommand final : public Command
	{
	public:
		GridMoveCommand(GameObject& actor, glm::ivec2 direction);
		void Execute() override;
	private:
		GameObject& m_Actor;
		glm::ivec2 m_Direction;
	};

	// Pushes the ice block immediately in front of Pengo (in facing direction)
	class PushCommand final : public Command
	{
	public:
		explicit PushCommand(GameObject& actor);
		void Execute() override;
	private:
		GameObject& m_Actor;
	};

	// Temporary for taking damage with button press
	class TakeDamageCommand final : public Command
	{
	public:
		explicit TakeDamageCommand(GameObject& actor, int damage = 1)
			: m_Actor{ actor }
			, m_Damage{ damage }
		{}

		void Execute() override;

	private:
		GameObject& m_Actor;
		int m_Damage;
	};

	// Temporary for increasing score with button press
	class IncreaseScoreCommand final : public Command
	{
	public:
		explicit IncreaseScoreCommand(GameObject& actor, int amount = 100)
			: m_Actor{ actor }
			, m_Amount{ amount }
		{}

		void Execute() override;

	private:
		GameObject& m_Actor;
		int m_Amount;
	};
}