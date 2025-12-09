#pragma once

#include "ClientGame.h"
#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "gui.h"
#include "menus/inLevel.h"
#include "menus/levelSelect.h"

class SingleplayerMenu : public NCL::CSC8503::PushdownState {
public:
  SingleplayerMenu(ClientGame &game) : game(game) {}

  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
      return Pop;
    }

    PushdownResult result = NoChange;

    auto &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    auto frame = NCL::gui::Frame("Singleplayer", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoDecoration);

    auto res = selectLevel(frame);

    if (res.has_value()) {
      game.SelectLevel(res.value());
      *newState = new InLevel(game);
      return Push;
    }

    if (frame.button("Back")) {
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
