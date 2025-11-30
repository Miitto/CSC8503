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

  enum class ServerState { Singleplayer, Host, Client };

protected:
  struct PingInfo {
    float timeSinceLastPing = 0.0f;
    std::chrono::steady_clock::time_point lastPingSentTime;
    int attemptsLeft = 3;
    std::function<void(ConnectionFailure)> cb;
    uint16_t messageType = static_cast<uint16_t>(BasicNetworkMessages::Ping);
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

    SetupPacketHandlers();

    return true;
  }

  void Connect(IP ip, std::function<void(ConnectionFailure)> cb) {
    net.emplace();
    if (!net->Connect(ip)) {
      cb(ConnectionFailure::ConnectionCreation);
      return;
    }

    SetupPacketHandlers();

    net->SendPacket(BasicNetworkMessages::Hello);
    pingInfo = {
        .lastPingSentTime = std::chrono::high_resolution_clock::now(),
        .cb = cb,
        .messageType = static_cast<uint16_t>(BasicNetworkMessages::Hello),
    };
  }

  void Disconnect();

  ServerState GetServerState() const {
    if (serverNet.has_value())
      return ServerState::Host;
    if (net.has_value())
      return ServerState::Client;
    return ServerState::Singleplayer;
  }

  void InitServer(uint16_t port, int maxClients);
  void ShutdownServer();

  void PingCheck(float dt);

  void SelectLevel(Level level);

  void UpdateGame(float dt) override;

  void ReceivePacket(GamePacketType type, GamePacket *payload,
                     int source) override;

protected:
  void NetworkUpdate(float dt) override;

  void SetupPacketHandlers();

  void StartLevel(Level level);

  int lastFullSync = 0;

  std::optional<PingInfo> pingInfo = std::nullopt;

  std::optional<GameClient> net = std::nullopt;

  std::optional<NCL::CSC8503::ServerCore> serverNet = std::nullopt;
};
} // namespace NCL::CSC8503
