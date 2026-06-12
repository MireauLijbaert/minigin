#include "Commands.h"
#include "TimeSingleton.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"

void dae::MovementCommand::Execute()
{
	m_direction = glm::normalize(m_direction); // Normalize the direction vector to ensure consistent movement speed regardless of the input magnitude.
	m_direction *= m_speed * dae::Time::GetInstance().GetDeltaTime(); // Scale the direction by the maximum speed and delta time to get the movement vector for this frame.
	m_actor->SetLocalPosition(m_actor->GetLocalPosition().GetPosition() + m_direction); // Update the owner's position by adding the movement vector to the current position.
}

void dae::TakeDamageCommand::Execute()
{
	auto health = m_Actor.GetComponent<HealthComponent>();
	if (health)
	{
		health->TakeDamage(m_Damage);
	}
}

void dae::IncreaseScoreCommand::Execute()
{
	auto score = m_Actor.GetComponent<ScoreComponent>();
	if (score)
	{
		score->AddScore(m_Amount);
	}
}
