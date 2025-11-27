#pragma once

#include "GameWorld.h"
#include "TutorialGame.h"
#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "gui.h"
#include "menus/pause.h"
#include "scenes/collisionTest.h"
#include "scenes/default.h"

class MainMenu : public NCL::CSC8503::PushdownState {
public:
  MainMenu(TutorialGame &game) : game(game) {}

  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
      (*newState) = new PauseMenu();
      return Push;
    }

    PushdownResult result = NoChange;

    auto &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    auto frame = NCL::gui::Frame("Main Menu", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoDecoration);
    if (frame.button("Default Scene")) {
      *newState = new DefaultScene(game);
      return Push;
    }
    if (frame.button("Collision Test")) {
      *newState = new CollisionTestScene(game);
      return Push;
    }
    if (frame.button("Exit to Desktop")) {
      NCL::Window::GetWindow()->RequestExit();
      return Pop;
    }
  }

  void OnAwake() override {
    NCL::Window::GetWindow()->ShowOSPointer(true);
    NCL::Window::GetWindow()->LockMouseToWindow(false);
  }

protected:
  TutorialGame &game;
};
