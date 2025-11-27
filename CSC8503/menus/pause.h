#pragma once

#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "gui.h"

class PauseMenu : public NCL::CSC8503::PushdownState {
public:
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

    auto frame = NCL::gui::Frame("Paused", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoDecoration);
    if (frame.button("Resume"))
      return Pop;
    if (frame.button("Quit to Main Menu")) {
      return Reset;
    }
    if (frame.button("Exit to Desktop")) {
      NCL::Window::GetWindow()->RequestExit();
      return Pop;
    }

    return result;
  }

  void OnInit() override { OnAwake(); }

  void OnAwake() override {
    NCL::Window::GetWindow()->ShowOSPointer(true);
    NCL::Window::GetWindow()->LockMouseToWindow(false);
  }
};