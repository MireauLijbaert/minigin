#include "PengoStates.h"

void PengoIdleState::OnEnter() { /* play idle animation */ }
void PengoIdleState::Update() { /* check for input, transition to Moving or Pushing */ }
void PengoIdleState::OnExit() {}

void PengoMovingState::OnEnter() { /* play walk animation */ }
void PengoMovingState::Update() { /* move, check if stopped → Idle, check if hitting block → Pushing */ }
void PengoMovingState::OnExit() {}

void PengoPushingState::OnEnter() { /* play push animation */ }
void PengoPushingState::Update() { /* wait for anim to finish, check block vs wall, → Idle when done */ }
void PengoPushingState::OnExit() {}

void PengoDyingState::OnEnter() { /* play death animation, disable input */ }
void PengoDyingState::Update() { /* wait for anim to finish, notify game */ }
void PengoDyingState::OnExit() {}