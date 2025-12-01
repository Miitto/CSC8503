#include "NetworkBase.h"
#include "./enet/enet.h"
#include "logging/logger.h"

NetworkBase::NetworkBase() { netHandle = nullptr; }

NetworkBase::~NetworkBase() {
  if (netHandle) {
    enet_host_destroy(netHandle);
  }
}

void NetworkBase::Initialise() { enet_initialize(); }

void NetworkBase::Destroy() { enet_deinitialize(); }

bool NetworkBase::ProcessPacket(GamePacket *packet, int peerID) {
  auto type = packet->type;
  NET_TRACE("Recieved packet of type {} from peer {}", type, peerID);
  auto iters = GetPacketHandlers(type);
  if (!iters.has_value()) {
    NET_DEBUG("No handlers for packet type {}", type);
    return false;
  }

  // TODO: WTF, using a ranged for loop here causes iterator Orphan_me_v3 to
  // access violate?? Only if logging is set to DEBUG, TRACE is fine

  for (auto it = iters->first; it != iters->second; ++it) {
    auto &handler = *it;
    handler.second->ReceivePacket(type, packet, peerID);
  }

  return true;
}