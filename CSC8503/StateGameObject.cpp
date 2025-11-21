#include "StateGameObject.h"
#include "ai/state_machine/State.h"
#include "ai/state_machine/StateMachine.h"
#include "ai/state_machine/StateTransition.h"
#include "physics/PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject()
    : stateMachine(std::make_unique<StateMachine>()) {
  auto A = std::make_unique<State>([&](float dt) { MoveLeft(dt); });
  auto B = std::make_unique<State>([&](float dt) { MoveRight(dt); });

  auto stateAToB = std::make_unique<StateTransition>(
      A.get(), B.get(), [&]() { return counter > 3.0f; });
  auto stateBToA = std::make_unique<StateTransition>(
      B.get(), A.get(), [&]() { return counter < 0.0f; });
  stateMachine->AddTransition(std::move(stateAToB));
  stateMachine->AddTransition(std::move(stateBToA));

  stateMachine->AddState(std::move(A));
  stateMachine->AddState(std::move(B));
}

void StateGameObject::Update(float dt) { stateMachine->Update(dt); }

void StateGameObject::MoveLeft(float dt) {
  GetPhysicsObject()->AddForce({-100, -0, 0});
  counter += dt;
}

void StateGameObject::MoveRight(float dt) {
  GetPhysicsObject()->AddForce({100, 0, 0});
  counter -= dt;
}