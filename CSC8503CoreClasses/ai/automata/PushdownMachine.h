#pragma once

namespace NCL {
namespace CSC8503 {
class PushdownState;

class PushdownMachine {
public:
  PushdownMachine(PushdownState *initialState);
  ~PushdownMachine();

  bool Update(float dt);
  void Reset();

protected:
  PushdownState *activeState;
  PushdownState *initialState;

  std::stack<PushdownState *> stateStack;
};
} // namespace CSC8503
} // namespace NCL
