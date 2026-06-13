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
    explicit SnoBeeChasingState(dae::SnoBeeComponent& snobee, float duration = 6.f);
    void OnEnter() override;
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
