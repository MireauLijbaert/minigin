#include "SnoBeeStates.h"
#include "SnoBeeComponent.h"
#include "GridMovementComponent.h"
#include "GridRegistry.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <climits>

// ---------------------------------------------------------------------------
// SnoBeeWanderState
// ---------------------------------------------------------------------------

SnoBeeWanderState::SnoBeeWanderState(dae::SnoBeeComponent& snobee, float duration)
    : m_SnoBee{ snobee }
    , m_Duration{ duration }
{}

void SnoBeeWanderState::OnEnter()
{
    // Random 3-9s so multiple sno-bees don't all chase at the same time
    m_Duration = 3.f + static_cast<float>(rand() % 61) / 10.f;
    m_Timer = 0.f;
    static const glm::ivec2 dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };
    m_CurrentDir = dirs[rand() % 4];
}

void SnoBeeWanderState::Update()
{
    m_Timer += dae::Time::GetInstance().GetDeltaTime();
    if (m_Timer >= m_Duration)
    {
        if (m_SnoBee.CanChase())
            m_SnoBee.RequestTransition(std::make_unique<SnoBeeChasingState>(m_SnoBee));
        else
            m_Timer = m_Duration * 0.5f; // retry after half a cycle
        return;
    }

    auto* movement = m_SnoBee.GetMovement();
    if (!movement || movement->IsMoving()) return;

    if (TryDirection(m_CurrentDir)) return;

    static const glm::ivec2 dirs[4] = { {1,0},{-1,0},{0,1},{0,-1} };
    int start = rand() % 4;
    for (int i = 0; i < 4; ++i)
    {
        glm::ivec2 dir = dirs[(start + i) % 4];
        if (dir == m_CurrentDir) continue;
        if (TryDirection(dir)) { m_CurrentDir = dir; return; }
    }

    // Completely walled in: break out (returns to wander, not chase)
    int breakStart = rand() % 4;
    for (int i = 0; i < 4; ++i)
    {
        glm::ivec2 dir = dirs[(breakStart + i) % 4];
        const glm::ivec2 target = movement->GetGridPos() + dir;
        const glm::ivec2 size   = m_SnoBee.GetGridSize();
        if (target.x < 0 || target.x >= size.x || target.y < 0 || target.y >= size.y) continue;
        if (!m_SnoBee.GetRegistry()->IsEmpty(target))
        {
            m_SnoBee.RequestTransition(
                std::make_unique<SnoBeeBreakingState>(m_SnoBee, target, m_SnoBee.GetBreakDuration(), false)
            );
            return;
        }
    }
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
    m_SnoBee.NotifyChaseStart();
}

void SnoBeeChasingState::OnExit()
{
    m_SnoBee.NotifyChaseEnd();
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

    const glm::ivec2 primaryTarget = myPos + primary;
    const bool primaryInBounds = (primaryTarget.x >= 0 && primaryTarget.x < size.x &&
                                  primaryTarget.y >= 0 && primaryTarget.y < size.y);

    // --- FRENZY: bulldoze straight, break everything ---
    if (m_SnoBee.IsFrenzy())
    {
        if (primaryInBounds)
        {
            if (registry->IsEmpty(primaryTarget))
                movement->SetDirection(primary);
            else
                m_SnoBee.RequestTransition(
                    std::make_unique<SnoBeeBreakingState>(m_SnoBee, primaryTarget, m_SnoBee.GetBreakDuration(), true)
                );
        }
        return;
    }

    // --- NORMAL: navigate around; only break if it saves 3+ cells ---
    int bestFreeDist = INT_MAX;
    glm::ivec2 bestFreeDir{};
    for (auto dir : candidates)
    {
        const glm::ivec2 target = myPos + dir;
        if (target.x < 0 || target.x >= size.x || target.y < 0 || target.y >= size.y) continue;
        if (!registry->IsEmpty(target)) continue;
        const glm::ivec2 nd = playerPos - target;
        const int d = std::abs(nd.x) + std::abs(nd.y);
        if (d < bestFreeDist) { bestFreeDist = d; bestFreeDir = dir; }
    }

    if (primaryInBounds && !registry->IsEmpty(primaryTarget))
    {
        const glm::ivec2 nd = playerPos - primaryTarget;
        const int breakDist = std::abs(nd.x) + std::abs(nd.y);
        // Break only when truly no free alternative, or block gives a big shortcut (3+ cells)
        if (bestFreeDist == INT_MAX || breakDist < bestFreeDist - 2)
        {
            m_SnoBee.RequestTransition(
                std::make_unique<SnoBeeBreakingState>(m_SnoBee, primaryTarget, m_SnoBee.GetBreakDuration(), true)
            );
            return;
        }
    }

    if (bestFreeDist < INT_MAX) { movement->SetDirection(bestFreeDir); return; }

    // Truly stuck (no free path at all): last resort break
    if (primaryInBounds && !registry->IsEmpty(primaryTarget))
    {
        m_SnoBee.RequestTransition(
            std::make_unique<SnoBeeBreakingState>(m_SnoBee, primaryTarget, m_SnoBee.GetBreakDuration(), true)
        );
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

// ---------------------------------------------------------------------------
// SnoBeeBreakingState
// ---------------------------------------------------------------------------

SnoBeeBreakingState::SnoBeeBreakingState(dae::SnoBeeComponent& snobee, glm::ivec2 targetCell, float duration, bool backToChase)
    : m_SnoBee{ snobee }
    , m_TargetCell{ targetCell }
    , m_Duration{ duration }
    , m_BackToChase{ backToChase }
{}

void SnoBeeBreakingState::OnEnter()
{
    m_Timer = 0.f;
}

void SnoBeeBreakingState::Update()
{
    // Block already gone (Pengo pushed it): resume appropriate state
    if (m_SnoBee.GetRegistry()->IsEmpty(m_TargetCell))
    {
        if (m_BackToChase)
            m_SnoBee.RequestTransition(std::make_unique<SnoBeeChasingState>(m_SnoBee));
        else
            m_SnoBee.RequestTransition(std::make_unique<SnoBeeWanderState>(m_SnoBee));
        return;
    }

    m_Timer += dae::Time::GetInstance().GetDeltaTime();
    if (m_Timer < m_Duration) return;

    auto* blockObj = m_SnoBee.GetRegistry()->GetAt(m_TargetCell);
    if (blockObj)
    {
        m_SnoBee.GetRegistry()->Unregister(m_TargetCell);
        blockObj->MarkForRemoval();
    }

    if (m_BackToChase)
        m_SnoBee.RequestTransition(std::make_unique<SnoBeeChasingState>(m_SnoBee));
    else
        m_SnoBee.RequestTransition(std::make_unique<SnoBeeWanderState>(m_SnoBee));
}
