#pragma once
#include "NetworkedGame.h"
#include "networking/GameClient.h"
#include "networking/NetworkBase.h"
#include "networking/ip.h"
#include "serverCore.h"

namespace NCL::CSC8503 {
class GameServer;
class GameClient;
class NetworkPlayer;
class NetworkObject;

class ClientGame : public NetworkedGame {
public:
  enum class ConnectionFailure {
    None,
    ConnectionCreation,
    NoPingResponse,
  };

protected:
  struct PingInfo {
    float timeSinceLastPing = 0.0f;
    std::chrono::steady_clock::time_point lastPingSentTime;
    int attemptsLeft = 3;
    std::function<void(ConnectionFailure)> cb;
  };

public:
  ClientGame(GameWorld &gameWorld, GameTechRendererInterface &renderer,
             PhysicsSystem &physics);

  template <typename S, typename F> struct ConnectionCallbacks {
    S onSuccess;
    F onFailure;
  };

  bool Connect(IP ip) {
    net.emplace();
    if (!net->Connect(ip))
      return false;

    net->RegisterPacketHandler(BasicNetworkMessages::Delta_State, this);
    net->RegisterPacketHandler(BasicNetworkMessages::Full_State, this);
    net->RegisterPacketHandler(BasicNetworkMessages::Player_Connected, this);
    net->RegisterPacketHandler(BasicNetworkMessages::Player_Disconnected, this);

    return true;
  }

  void Connect(IP ip, std::function<void(ConnectionFailure)> cb) {
    net.emplace();
    if (!net->Connect(ip)) {
      cb(ConnectionFailure::ConnectionCreation);
      return;
    }

    net->RegisterPacketHandler(BasicNetworkMessages::Delta_State, this);
    net->RegisterPacketHandler(BasicNetworkMessages::Full_State, this);
    net->RegisterPacketHandler(BasicNetworkMessages::Player_Connected, this);
    net->RegisterPacketHandler(BasicNetworkMessages::Player_Disconnected, this);
    net->RegisterPacketHandler(BasicNetworkMessages::Ping_Response, this);

    net->SendPacket(BasicNetworkMessages::Ping);
    pingInfo = {
        .lastPingSentTime = std::chrono::high_resolution_clock::now(),
        .cb = cb,
    };
  }

  void Disconnect();

  void InitServer(uint16_t port, int maxClients);
  void ShutdownServer();

  void PingCheck(float dt);

  void StartLevel();

  void ReceivePacket(GamePacketType type, GamePacket *payload,
                     int source) override;

protected:
  void NetworkUpdate(float dt) override;

  int lastFullSync = 0;

  std::optional<PingInfo> pingInfo = std::nullopt;

  std::optional<GameClient> net = std::nullopt;

  std::optional<NCL::CSC8503::ServerCore> serverNet = std::nullopt;
};
} // namespace NCL::CSC8503
