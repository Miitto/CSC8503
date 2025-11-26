#pragma once

#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "imgui/imgui.h"

class PauseMenu : public NCL::CSC8503::PushdownState {
public:
  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
      return Pop;
    }

    PushdownResult result = NoChange;

    ImGui::Begin("Paused", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoDecoration);
    if (ImGui::Button("Resume"))
      return = Pop;
    if (ImGui::Button("Quit to Main Menu")) {
      result = Reset;
    }
    if (ImGui::Button("Exit to Desktop")) {
      NCL::Window::GetWindow()->RequestExit();
      result = Pop;
    }

    ImGui::End();

    return result;
  }

  void OnAwake() override {
    NCL::Window::GetWindow()->ShowOSPointer(true);
    NCL::Window::GetWindow()->LockMouseToWindow(false);
  }

  void OnSleep() override {
    NCL::Window::GetWindow()->ShowOSPointer(false);
    NCL::Window::GetWindow()->LockMouseToWindow(true);
  }
};