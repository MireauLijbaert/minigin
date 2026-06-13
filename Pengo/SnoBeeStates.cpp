#include "SnoBeeStates.h"
#include "SnoBeeComponent.h"
#include "GridMovementComponent.h"
#include "GridRegistry.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <cstdlib>
#include <cmath>

// ---------------------------------------------------------------------------
// SnoBeeWanderState
// ---------------------------------------------------------------------------

SnoBeeWanderState::SnoBeeWanderState(dae::SnoBeeComponent& snobee, float duration)
    : m_SnoBee{ snobee }
    , m_Duration{ duration }
{}

void SnoBeeWanderState::OnEnter()
{
    m_Timer = 0.f;
    static const glm::ivec2 dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };
    m_CurrentDir = dirs[rand() % 4];
}

void SnoBeeWanderState::Update()
{
    m_Timer += dae::Time::GetInstance().GetDeltaTime();
    if (m_Timer >= m_Duration)
    {
        m_SnoBee.RequestTransition(std::make_unique<SnoBeeChasingState>(m_SnoBee));
        return;
    }

    auto* movement = m_SnoBee.GetMovement();
    if (!movement || movement->IsMoving()) return;

    // Try current direction first; if blocked rotate through the other three
    if (TryDirection(m_CurrentDir)) return;

    static const glm::ivec2 dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };
    // Shuffle start index so we don't always prefer the same fallback
    int start = rand() % 4;
    for (int i = 0; i < 4; ++i)
    {
        glm::ivec2 dir = dirs[(start + i) % 4];
        if (dir == m_CurrentDir) continue;
        if (TryDirection(dir))
        {
            m_CurrentDir = dir;
            return;
        }
    }
    // All directions blocked: stay put (shouldn't happen on an open grid)
}

bool SnoBeeWanderState::TryDirection(glm::ivec2 dir)
{
    auto* movement = m_SnoBee.GetMovement();
    if (!movement) return false;

    const glm::ivec2 target = movement->GetGridPos() + dir;
    const glm::ivec2 size   = m_SnoBee.GetGridSize();

    if (target.x < 0 || target.x >= size.x || target.y < 0 || target.y >= size.y)
        return false;

    if (!m_SnoBee.GetRegistry()->IsEmpty(target))
        return false;

    movement->SetDirection(dir);
    return true;
}

// ---------------------------------------------------------------------------
// SnoBeeChasingState
// ---------------------------------------------------------------------------

SnoBeeChasingState::SnoBeeChasingState(dae::SnoBeeComponent& snobee, float duration)
    : m_SnoBee{ snobee }
    , m_Duration{ duration }
{}

void SnoBeeChasingState::OnEnter()
{
    m_Timer = 0.f;
}

void SnoBeeChasingState::Update()
{
    m_Timer += dae::Time::GetInstance().GetDeltaTime();
    if (m_Timer >= m_Duration)
    {
        m_SnoBee.RequestTransition(std::make_unique<SnoBeeWanderState>(m_SnoBee));
        return;
    }

    auto* movement = m_SnoBee.GetMovement();
    if (!movement || movement->IsMoving()) return;

    auto* player = m_SnoBee.GetPlayer();
    if (!player) return;

    auto* playerMov = player->GetComponent<dae::GridMovementComponent>();
    if (!playerMov) return;

    const glm::ivec2 myPos     = movement->GetGridPos();
    const glm::ivec2 playerPos = playerMov->GetGridPos();
    const glm::ivec2 diff      = playerPos - myPos;

    // Primary axis: whichever distance is larger
    glm::ivec2 primary, secondary;
    if (std::abs(diff.x) >= std::abs(diff.y))
    {
        primary   = { diff.x > 0 ? 1 : -1, 0 };
        secondary = diff.y != 0 ? glm::ivec2{ 0, diff.y > 0 ? 1 : -1 } : glm::ivec2{ 0, 1 };
    }
    else
    {
        primary   = { 0, diff.y > 0 ? 1 : -1 };
        secondary = diff.x != 0 ? glm::ivec2{ diff.x > 0 ? 1 : -1, 0 } : glm::ivec2{ 1, 0 };
    }

    const glm::ivec2 candidates[4] = { primary, secondary, -secondary, -primary };

    const glm::ivec2 size     = m_SnoBee.GetGridSize();
    auto*            registry = m_SnoBee.GetRegistry();

    for (auto dir : candidates)
    {
        const glm::ivec2 target = myPos + dir;
        if (target.x < 0 || target.x >= size.x || target.y < 0 || target.y >= size.y) continue;
        if (!registry->IsEmpty(target)) continue;

        movement->SetDirection(dir);
        break;
    }
}

// ---------------------------------------------------------------------------
// SnoBeeStunnedState
// ---------------------------------------------------------------------------

SnoBeeStunnedState::SnoBeeStunnedState(dae::SnoBeeComponent& snobee, float duration)
    : m_SnoBee{ snobee }
    , m_Duration{ duration }
{}

void SnoBeeStunnedState::Update()
{
    m_Timer += dae::Time::GetInstance().GetDeltaTime();
    if (m_Timer >= m_Duration)
        m_SnoBee.RequestTransition(std::make_unique<SnoBeeWanderState>(m_SnoBee));
}
