#include "StateGameObject.h"
#include "ai/state_machine/State.h"
#include "ai/state_machine/StateMachine.h"
#include "ai/state_machine/StateTransition.h"
#include "physics/PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

StateGameObject::StateGameObject() {}

StateGameObject::~StateGameObject() { delete stateMachine; }

void StateGameObject::Update(float dt) {}

void StateGameObject::MoveLeft(float dt) {}

void StateGameObject::MoveRight(float dt) {}