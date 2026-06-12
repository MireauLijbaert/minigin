#include "StateMachine.h"

void StateMachine::SetState(std::unique_ptr<BaseState> newState) {
    if (m_CurrentState) m_CurrentState->OnExit();
    m_CurrentState = std::move(newState);
    if (m_CurrentState) m_CurrentState->OnEnter();
}

void StateMachine::Update() {
    if (m_CurrentState) m_CurrentState->Update();
}