#pragma once

#include "ClientGame.h"
#include "Window.h"
#include "ai/automata/PushdownState.h"
#include "gui.h"
#include "scenes/collisionTest.h"
#include "scenes/default.h"

class JoinMenu : public NCL::CSC8503::PushdownState {
public:
  JoinMenu(ClientGame &game) : game(game) {}

  PushdownResult OnUpdate(float dt,
                          NCL::CSC8503::PushdownState **newState) override {
    if (connecting) {
      ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
                                     ImGui::GetIO().DisplaySize.y * 0.5f),
                              ImGuiCond_Always, ImVec2(0.5f, 0.5f));

      game.PingCheck(dt);

      auto frame = NCL::gui::Frame("Connecting", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoDecoration);
      if (errorMsg.has_value()) {
        frame.text("Error: %s", errorMsg->data());
        if (frame.button("Ok")) {
          errorMsg = std::nullopt;
          connecting = false;
        }
      } else {
        frame.text("Connecting to server...");
      }
      return NoChange;
    }

    if (NCL::Window::GetKeyboard()->KeyPressed(NCL::KeyCodes::ESCAPE)) {
      return Pop;
    }

    PushdownResult result = NoChange;

    auto &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(
        ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    auto frame = NCL::gui::Frame("Join", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoDecoration);

    frame.text("Enter Server IP:");
    static uint8_t ipParts[4] = {127, 0, 0, 1};
    static uint16_t port = NetworkBase::GetDefaultPort();

    float width = ImGui::CalcTextSize("127").x + 10.0f;
    float portWidth = ImGui::CalcTextSize("12345").x + 10.0f;

    frame.width(width).input("###IPPart1", ipParts[0], 0, 0);
    frame.sameLine().text(".").sameLine().width(width).input("###IPPart2",
                                                             ipParts[1], 0, 0);
    frame.sameLine().text(".").sameLine().width(width).input("###IPPart3",
                                                             ipParts[2], 0, 0);
    frame.sameLine().text(".").sameLine().width(width).input("###IPPart4",
                                                             ipParts[3], 0, 0);
    frame.sameLine().text(":").sameLine().width(portWidth).input("###Port",
                                                                 port, 0, 0);

    auto connectCb = [&](ClientGame::ConnectionFailure failure) {
      switch (failure) {
      case ClientGame::ConnectionFailure::ConnectionCreation: {
        errorMsg = "Failed to create connection to server.";
        break;
      }
      case ClientGame::ConnectionFailure::NoPingResponse: {
        errorMsg = "No response from server.";
        break;
      }
      case ClientGame::ConnectionFailure::None: {
        connecting = false;
        *newState = new DefaultScene(game);
        result = Push;
      }
      }
    };

    if (frame.button("Connect")) {
      game.Connect({ipParts[0], ipParts[1], ipParts[2], ipParts[3], port},
                   connectCb);
      connecting = true;
    }

    if (frame.button("Back")) {
      return Pop;
    }

    return result;
  }

  void OnAwake() override {
    NCL::Window::GetWindow()->ShowOSPointer(true);
    NCL::Window::GetWindow()->LockMouseToWindow(false);
    errorMsg = std::nullopt;
    game.SetActive(false);
  }

protected:
  ClientGame &game;

  bool connecting = false;
  std::optional<std::string_view> errorMsg = std::nullopt;
};
