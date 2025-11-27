#pragma once

#include "GameWorld.h"
#include "TutorialGame.h"
#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "imgui/imgui.h"
#include "menus/pause.h"

class IScene : public NCL::CSC8503::PushdownState {
public:
  IScene(TutorialGame &game) : game(game) {}

  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
      *newState = new PauseMenu();
      return Push;
    }

    game.UpdateGame(dt);

    return NoChange;
  }

  void OnAwake() override {
    NCL::Window::GetWindow()->ShowOSPointer(false);
    NCL::Window::GetWindow()->LockMouseToWindow(true);
  }

  void OnDestroy() override { game.Clear(); }

protected:
  TutorialGame &game;
};
