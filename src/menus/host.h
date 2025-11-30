#pragma once

#include "ClientGame.h"
#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "gui.h"
#include "menus/hostGuard.h"
#include "menus/singleplayer.h"

class HostMenu : public NCL::CSC8503::PushdownState {
public:
  HostMenu(ClientGame &game) : game(game) {}

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

    auto frame = NCL::gui::Frame("Host", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoDecoration);
    static uint16_t port = NetworkBase::GetDefaultPort();
    static int maxClients = 4;
    frame.input("Port:", port, 1, 100, 0);
    frame.input("Max Players:", maxClients, 1, 10, 0);

    if (frame.button("Start Server")) {
      game.InitServer(port, maxClients);
      *newState = new HostGuard(game, new SingleplayerMenu(game));
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
