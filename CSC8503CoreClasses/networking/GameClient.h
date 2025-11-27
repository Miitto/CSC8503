#pragma once
#include "NetworkBase.h"
#include <atomic>
#include <stdint.h>
#include <thread>

#include "ip.h"

namespace NCL {
namespace CSC8503 {
class GameObject;
class GameClient : public NetworkBase {
public:
  GameClient();
  ~GameClient();

  bool Connect(IP ip);

  void SendPacket(GamePacket &payload);
  void SendPacket(GamePacket &&payload);

  void UpdateClient();

protected:
  _ENetPeer *netPeer;
};
} // namespace CSC8503
} // namespace NCL
