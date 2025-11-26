#pragma once

#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "imgui/imgui.h"
#include "menus/pause.h"

class MainMenu : public NCL::CSC8503::PushdownState {
public:
  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
      newState = new PauseMenu();
      return Push;
    }

    PushdownResult result = NoChange;

    ImGui::Begin("Paused", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize |
                     ImGuiWindowFlags_NoDecoration);
    if (ImGui::Button("Resume"))
      result = Pop;
    if (ImGui::Button("Exit to Desktop")) {
      NCL::Window::GetWindow()->RequestExit();
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
