#pragma once
#include "GameObject.h"
#include "ai/state_machine/StateMachine.h"

namespace NCL::CSC8503 {
class StateMachine;
}

class Oscillator : public NCL::CSC8503::GameObject {
public:
  struct Keyframe {
    NCL::Maths::Vector3 position;
    NCL::Maths::Quaternion orientation;
  };

  Oscillator(Keyframe start, Keyframe end, float duration);
  ~Oscillator() = default;

  virtual void Update(float dt) override;

protected:
  std::unique_ptr<NCL::CSC8503::StateMachine> stateMachine;
  float counter = 0.0f;

  void updatePosition();

  Keyframe start;
  Keyframe end;
  float duration;
};
