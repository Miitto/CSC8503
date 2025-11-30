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
      activeState->OnDestroy();
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
      activeState->OnInit();
    } break;
    case PushdownState::Reset: {
      Reset();
    } break;
    case PushdownState::Replace: {
      activeState->OnDestroy();
      activeState = newState;
      stateStack.pop();
      stateStack.push(activeState);
      activeState->OnInit();
    } break;
    case PushdownState::ReplaceBelow: {
      activeState->OnDestroy();
      delete activeState;
      stateStack.pop();
      auto topState = stateStack.top();
      topState->OnDestroy();
      delete topState;
      stateStack.pop();
      stateStack.push(newState);
      activeState = stateStack.top();
      activeState->OnInit();
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
    s->OnDestroy();
    delete s;
    stateStack.pop();
  }
  if (stateStack.empty())
    return;

  activeState = stateStack.top();
  activeState->OnAwake();

  assert(stateStack.size() == 1 && "PushdownMachine::Reset: Stack size != 1");
  assert(activeState == initialState &&
         "PushdownMachine::Reset: Active state != initial state");
}