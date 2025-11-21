#pragma once

namespace NCL {
namespace CSC8503 {
typedef std::function<void(float)> StateUpdateFunction;

class State {
public:
  State() = default;
  ~State() = default;

  State(StateUpdateFunction someFunc) { func = someFunc; }
  void Update(float dt) {
    if (func != nullptr) {
      func(dt);
    }
  }

protected:
  StateUpdateFunction func;
};
} // namespace CSC8503
} // namespace NCL