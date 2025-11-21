#include "StateMachine.h"
#include "State.h"
#include "StateTransition.h"

using namespace NCL::CSC8503;

void StateMachine::AddState(StateP &&s) {
  if (activeState == nullptr) {
    activeState = s.get();
  }

  allStates.emplace_back(std::move(s));
}

void StateMachine::AddTransition(StateTransitionP &&t) {
  auto p = std::make_pair(t->GetSourceState(), std::move(t));
  allTransitions.insert(std::move(p));
}

void StateMachine::Update(float dt) {
  if (activeState) {
    activeState->Update(dt);

    // Get the transition set starting from this state node;
    std::pair<TransitionIterator, TransitionIterator> range =
        allTransitions.equal_range(activeState);

    for (auto &i = range.first; i != range.second; ++i) {
      if (i->second->CanTransition()) {
        State *newState = i->second->GetDestinationState();
        activeState = newState;
      }
    }
  }
}