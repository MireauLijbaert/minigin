#pragma once
#include <memory>
#include "BaseState.h"

class StateMachine {
    std::unique_ptr<BaseState> m_CurrentState;
public:
    void SetState(std::unique_ptr<BaseState> newState);
    void Update(float deltaTime);
};