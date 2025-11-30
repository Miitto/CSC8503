#pragma once

#include "ClientGame.h"
#include "ai/automata/PushdownState.h"
#include "gui.h"
#include <optional>

std::optional<Level> selectLevel(NCL::gui::Frame &frame) {
  if (frame.button("Default"))
    return Level::Default;
  if (frame.button("Collision Test"))
    return Level::CollisionTest;

  return std::nullopt;
}

class LevelSelectMenu : public NCL::CSC8503::PushdownState {
public:
  LevelSelectMenu(ClientGame &game) : game(game) {}

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
    auto frame = NCL::gui::Frame("Level Select", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoDecoration);
    auto res = selectLevel(frame);

    if (res.has_value()) {
      game.SelectLevel(*res);
      return Pop;
    }

    if (frame.button("Back")) {
      return Pop;
    }
  }

protected:
  ClientGame &game;
};