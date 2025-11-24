#include "Oscillator.h"

#include "ai/state_machine/State.h"
#include "ai/state_machine/StateMachine.h"
#include "ai/state_machine/StateTransition.h"

using namespace NCL;
using namespace CSC8503;

Oscillator::Oscillator(Keyframe start, Keyframe end, float duration)
    : GameObject("Oscillator"), start(start), end(end), duration(duration) {
  auto A = std::make_unique<State>([&](float dt) {
    updatePosition();
    counter += dt;
  });
  auto B = std::make_unique<State>([&](float dt) {
    updatePosition();
    counter -= dt;
  });

  auto stateAToB = std::make_unique<StateTransition>(
      A.get(), B.get(), [&]() { return counter > this->duration; });
  auto stateBToA = std::make_unique<StateTransition>(
      B.get(), A.get(), [&]() { return counter < 0.0f; });
  stateMachine->AddTransition(std::move(stateAToB));
  stateMachine->AddTransition(std::move(stateBToA));

  stateMachine->AddState(std::move(A));
  stateMachine->AddState(std::move(B));
}

void Oscillator::Update(float dt) { stateMachine->Update(dt); }

void Oscillator::updatePosition() {
  float blend = std::clamp(counter / duration, 0.0f, 1.0f);
  auto pos = NCL::Maths::Vector::Lerp(start.position, end.position, blend);
  auto rot =
      NCL::Maths::Quaternion::Slerp(start.orientation, end.orientation, blend);
  GetTransform().SetPosition(pos);
  GetTransform().SetOrientation(rot);
}