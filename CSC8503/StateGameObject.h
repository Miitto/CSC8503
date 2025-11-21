#pragma once
#include "GameObject.h"

namespace NCL {
namespace CSC8503 {
class StateMachine;
class StateGameObject : public GameObject {
public:
  StateGameObject();
  ~StateGameObject() = default;

  virtual void Update(float dt) override;

protected:
  void MoveLeft(float dt);
  void MoveRight(float dt);

  std::unique_ptr<StateMachine> stateMachine;
  float counter = 0.0f;
};
} // namespace CSC8503
} // namespace NCL
