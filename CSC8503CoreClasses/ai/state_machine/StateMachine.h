#pragma once

namespace NCL {
namespace CSC8503 {
class State;
class StateTransition;

using StateP = std::unique_ptr<State>;
using StateTransitionP = std::unique_ptr<StateTransition>;

using TransitionContainer = std::multimap<State *, StateTransitionP>;
using TransitionIterator = TransitionContainer::iterator;

class StateMachine {
public:
  StateMachine() = default;
  virtual ~StateMachine() = default; // made it virtual!

  void AddState(StateP &&s);
  void AddTransition(StateTransitionP &&t);

  virtual void Update(float dt); // made it virtual!

protected:
  State *activeState = nullptr;

  std::vector<StateP> allStates;
  TransitionContainer allTransitions;
};
} // namespace CSC8503
} // namespace NCL