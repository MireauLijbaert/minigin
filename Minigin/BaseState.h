#pragma once

class BaseState {
public:
    virtual ~BaseState() = default;
    virtual void OnEnter() {}
    virtual void Update(float deltaTime) = 0;
    virtual void OnExit() {}
};