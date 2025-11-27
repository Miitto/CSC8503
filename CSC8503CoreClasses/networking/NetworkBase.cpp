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
  auto iters = GetPacketHandlers(packet->type);
  if (!iters.has_value()) {
    NET_DEBUG("No handlers for packet type {}", packet->type);
    return false;
  }

  for (auto &handler : *iters) {
    handler.second->ReceivePacket(packet->type, packet, peerID);
  }

  return true;
}