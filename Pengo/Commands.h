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