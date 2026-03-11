#include "Commands.h"
#include "MovementComponent.h"

void dae::MovementCommand::Execute()
{
	if (m_actor)
	{
		MovementComponent* movementComponent = m_actor->GetComponent<MovementComponent>();
		if (movementComponent)
		{
			movementComponent->Move(m_direction);
		}
	}
}

