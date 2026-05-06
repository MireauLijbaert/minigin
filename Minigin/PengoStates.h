#pragma once
#include "BaseState.h"

class PengoIdleState final : public BaseState {
public:
    void OnEnter() override;
    void Update(float deltaTime) override;
    void OnExit() override;
};

class PengoMovingState final : public BaseState {
public:
    void OnEnter() override;
    void Update(float deltaTime) override;
    void OnExit() override;
};

class PengoPushingState final : public BaseState {
public:
    void OnEnter() override;
    void Update(float deltaTime) override;
    void OnExit() override;
};

class PengoDyingState final : public BaseState {
public:
    void OnEnter() override;
    void Update(float deltaTime) override;
    void OnExit() override;
};