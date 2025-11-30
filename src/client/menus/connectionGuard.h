#pragma once

#include "ClientGame.h"
#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "gui.h"
#include "scenes/collisionTest.h"
#include "scenes/default.h"

class ConnectionGuard : public NCL::CSC8503::PushdownState {
public:
  ConnectionGuard(ClientGame &game, NCL::CSC8503::PushdownState *setState)
      : game(game), nextState(setState) {}

  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (nextState != nullptr) {
      *newState = nextState;
      return Push;
    }

    return shouldPop ? Pop : NoChange;
  }

  void OnAwake() override { shouldPop = true; }

  void OnDestroy() override {
    game.Disconnect();
    game.Clear();
  }

protected:
  ClientGame &game;
  bool shouldPop = false;
  NCL::CSC8503::PushdownState *nextState = nullptr;
};
