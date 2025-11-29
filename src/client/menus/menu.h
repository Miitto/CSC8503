#pragma once

#include "ClientGame.h"
#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "gui.h"
#include "menus/multiplayer.h"
#include "menus/pause.h"
#include "menus/solo.h"

class MainMenu : public NCL::CSC8503::PushdownState {
public:
  MainMenu(ClientGame &game) : game(game) {}

  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
      (*newState) = new PauseMenu(game);
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
    if (frame.button("Singleplayer")) {
      *newState = new SoloMenu(game);
      return Push;
    }
    if (frame.button("Multiplayer")) {
      *newState = new MultiplayerMenu(game);
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

    game.SetActive(false);
  }

protected:
  ClientGame &game;
};
