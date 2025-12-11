#pragma once

#include "GameWorld.h"
#include "TutorialGame.h"
#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "imgui/imgui.h"
#include "menus/pause.h"

class InLevel : public NCL::CSC8503::PushdownState {
public:
  InLevel(ClientGame &game) : game(game) {}

  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
      *newState = new PauseMenu(game);
      return Push;
    }

    if (!game.InLevel()) {
      return Pop;
    }

    return NoChange;
  }

  void OnInit() override { OnAwake(); }

  void OnAwake() override {
    NCL::Window::GetWindow()->ShowOSPointer(false);
    NCL::Window::GetWindow()->LockMouseToWindow(true);
    game.SetActive(true);
    game.SetCameraActive(true);
  }

  void OnDestroy() override { game.Clear(); }

protected:
  ClientGame &game;
};
