#pragma once
#include "BaseState.h"

namespace dae { class PengoControllerComponent; }

class PengoIdleState final : public BaseState {
    dae::PengoControllerComponent& m_Controller;
public:
    explicit PengoIdleState(dae::PengoControllerComponent& controller) : m_Controller(controller) {}
    void OnEnter() override;
    void Update() override;
    void OnExit() override;
};

class PengoMovingState final : public BaseState {
    dae::PengoControllerComponent& m_Controller;
public:
    explicit PengoMovingState(dae::PengoControllerComponent& controller) : m_Controller(controller) {}
    void OnEnter() override;
    void Update() override;
    void OnExit() override;
};

class PengoPushingState final : public BaseState {
    dae::PengoControllerComponent& m_Controller;
public:
    explicit PengoPushingState(dae::PengoControllerComponent& controller) : m_Controller(controller) {}
    void OnEnter() override;
    void Update() override;
    void OnExit() override;
};

class PengoDyingState final : public BaseState {
    dae::PengoControllerComponent& m_Controller;
public:
    explicit PengoDyingState(dae::PengoControllerComponent& controller) : m_Controller(controller) {}
    void OnEnter() override;
    void Update() override;
    void OnExit() override;
};