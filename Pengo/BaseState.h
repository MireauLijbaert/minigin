#pragma once

class BaseState {
public:
    virtual ~BaseState() = default;
    virtual void OnEnter() {}
    virtual void Update() = 0;
    virtual void OnExit() {}
};