#pragma once
#include "BaseState.h"
#include <glm/glm.hpp>
#include <memory>

namespace dae { class SnoBeeComponent; }

class SnoBeeWanderState final : public BaseState
{
public:
    explicit SnoBeeWanderState(dae::SnoBeeComponent& snobee, float duration = 4.f);
    void OnEnter() override;
    void Update() override;

private:
    bool TryDirection(glm::ivec2 dir);

    dae::SnoBeeComponent& m_SnoBee;
    float m_Timer{ 0.f };
    float m_Duration;
    glm::ivec2 m_CurrentDir{ 1, 0 };
};

class SnoBeeChasingState final : public BaseState
{
public:
    explicit SnoBeeChasingState(dae::SnoBeeComponent& snobee, float duration = 8.f);
    void OnEnter() override;
    void OnExit() override;
    void Update() override;

private:
    dae::SnoBeeComponent& m_SnoBee;
    float m_Timer{ 0.f };
    float m_Duration;
};

class SnoBeeStunnedState final : public BaseState
{
public:
    SnoBeeStunnedState(dae::SnoBeeComponent& snobee, float duration = 2.f);
    void Update() override;

private:
    dae::SnoBeeComponent& m_SnoBee;
    float m_Timer{ 0.f };
    float m_Duration;
};

// backToChase: true when entered from SnoBeeChasingState, false when from SnoBeeWanderState
class SnoBeeBreakingState final : public BaseState
{
public:
    SnoBeeBreakingState(dae::SnoBeeComponent& snobee, glm::ivec2 targetCell, float duration, bool backToChase = true);
    void OnEnter() override;
    void Update() override;

private:
    dae::SnoBeeComponent& m_SnoBee;
    glm::ivec2 m_TargetCell;
    float m_Timer{ 0.f };
    float m_Duration;
    bool  m_BackToChase;
};
