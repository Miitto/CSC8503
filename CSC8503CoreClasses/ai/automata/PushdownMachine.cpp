#include "PushdownMachine.h"
#include "PushdownState.h"
#include <cassert>

using namespace NCL::CSC8503;

PushdownMachine::PushdownMachine(PushdownState *initialState)
    : initialState(initialState), activeState(nullptr) {}

PushdownMachine::~PushdownMachine() {}

bool PushdownMachine::Update(float dt) {
  if (activeState) {
    PushdownState *newState = nullptr;
    PushdownState::PushdownResult result = activeState->OnUpdate(dt, &newState);

    switch (result) {
    case PushdownState::Pop: {
      activeState->OnSleep();
      delete activeState;
      stateStack.pop();
      if (stateStack.empty()) {
        return false;
      } else {
        activeState = stateStack.top();
        activeState->OnAwake();
      }
    } break;
    case PushdownState::Push: {
      activeState->OnSleep();

      stateStack.push(newState);
      activeState = newState;
      activeState->OnAwake();
    } break;
    case PushdownState::Reset: {
      activeState->OnSleep();
      Reset();
    } break;
    }
  } else {
    stateStack.push(initialState);
    activeState = initialState;
    activeState->OnAwake();
  }
  return true;
}

void PushdownMachine::Reset() {
  while (stateStack.size() > 1) {
    auto s = stateStack.top();
    delete s;
    stateStack.pop();
  }
  if (stateStack.empty())
    return;

  activeState = stateStack.top();
  activeState->OnAwake();

  assert(stateStack.size() == 1, "PushdownMachine::Reset: Stack size != 1");
  assert(activeState == initialState,
         "PushdownMachine::Reset: Active state != initial state");
}