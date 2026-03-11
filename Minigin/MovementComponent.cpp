#include "MovementComponent.h"
#include "TimeSingleton.h"


void dae::MovementComponent::Move(glm::vec3& direction)
{
	direction = glm::normalize(direction); // Normalize the direction vector to ensure consistent movement speed regardless of the input magnitude.
	direction *= m_MaxSpeed * dae::Time::GetInstance().GetDeltaTime(); // Scale the direction by the maximum speed and delta time to get the movement vector for this frame.
	GetOwner()->SetLocalPosition(GetOwner()->GetLocalPosition().GetPosition() + direction); // Update the owner's position by adding the movement vector to the current position.
}


void dae::MovementComponent::Move(float x, float y, float z)
{
	glm::vec3 direction{ x, y, z };
	Move(direction);
}