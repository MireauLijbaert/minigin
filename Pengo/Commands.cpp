#include "Commands.h"
#include "TimeSingleton.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "GridMovementComponent.h"
#include "IceBlockComponent.h"
#include "GridRegistry.h"
#include "PengoControllerComponent.h"

// Brief lock so Pengo can't move/push again immediately (push animation)
static constexpr float kPushLockDuration = 0.3f;

void dae::MovementCommand::Execute()
{
	m_direction = glm::normalize(m_direction);
	m_direction *= m_speed * dae::Time::GetInstance().GetDeltaTime();
	m_actor->SetLocalPosition(m_actor->GetLocalPosition().GetPosition() + m_direction);
}

dae::GridMoveCommand::GridMoveCommand(GameObject& actor, glm::ivec2 direction)
	: m_Actor{ actor }
	, m_Direction{ direction }
{}

void dae::GridMoveCommand::Execute()
{
	if (auto* movement = m_Actor.GetComponent<GridMovementComponent>())
		movement->SetDirection(m_Direction);
}

dae::PushCommand::PushCommand(GameObject& actor)
	: m_Actor{ actor }
{}

void dae::PushCommand::Execute()
{
	auto* movement = m_Actor.GetComponent<GridMovementComponent>();
	if (!movement || movement->IsMoving() || movement->IsLocked()) return;

	auto* registry = movement->GetRegistry();
	if (!registry) return;

	const glm::ivec2 facingDir  = movement->GetFacingDirection();
	const glm::ivec2 targetCell = movement->GetGridPos() + facingDir;
	const glm::ivec2 gridSize   = movement->GetGridSize();

	// Out of bounds: Pengo bumped the border wall, stun nearby Sno-bees
	if (targetCell.x < 0 || targetCell.x >= gridSize.x ||
	    targetCell.y < 0 || targetCell.y >= gridSize.y)
	{
		if (auto* pengo = m_Actor.GetComponent<PengoControllerComponent>())
			pengo->WallStun();
		movement->LockFor(kPushLockDuration);
		return;
	}

	if (auto* obj = registry->GetAt(targetCell))
	{
		if (auto* block = obj->GetComponent<IceBlockComponent>())
		{
			block->TryPush(facingDir, registry, gridSize);
			movement->LockFor(kPushLockDuration);
		}
	}
}

void dae::TakeDamageCommand::Execute()
{
	auto health = m_Actor.GetComponent<HealthComponent>();
	if (health)
		health->LoseLife();
}

void dae::IncreaseScoreCommand::Execute()
{
	auto score = m_Actor.GetComponent<ScoreComponent>();
	if (score)
		score->AddScore(m_Amount);
}
