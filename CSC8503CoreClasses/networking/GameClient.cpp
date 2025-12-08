#include "GameClient.h"

#include "enet/enet.h"
#include <logging/logger.h>

using namespace NCL;
using namespace CSC8503;

GameClient::GameClient() {
  NET_DEBUG("Creating GameClient");
  netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
  NET_TRACE("Created GameClient at {}", (void *)netHandle);
}

GameClient::~GameClient() {
  if (netHandle == nullptr)
    return;

  NET_TRACE("Destroying GameClient at {}", (void *)netHandle);
  NET_DEBUG("Destroyed GameClient");
}

bool GameClient::Connect(IP ip) {
  NET_DEBUG("GameClient connecting to {}", ip);
  ENetAddress addr{
      .host = ip.getAddress(),
      .port = ip.port(),
  };

  netPeer = enet_host_connect(netHandle, &addr, 2, 0);
  NET_TRACE("GameClient connect peer: {}", (void *)netPeer);

  if (netPeer == nullptr) {
    NET_ERROR("GameClient failed to create ENet peer for {}", ip);
    return false;
  } else {
    NET_INFO("GameClient connected to {}", ip);
  }

  return netPeer != nullptr;
}

void GameClient::UpdateClient() {
  if (!netHandle) {
    return;
  }

  ENetEvent event;
  while (enet_host_service(netHandle, &event, 0) > 0) {
    switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT:
      NET_INFO("Connected to server!");
      break;
    case ENET_EVENT_TYPE_RECEIVE: {
      GamePacket *packet = reinterpret_cast<GamePacket *>(event.packet->data);
      ProcessPacket(packet, 0);
      break;
    }
    case ENET_EVENT_TYPE_DISCONNECT:
      NET_INFO("Disconnected from server!");
      break;
    default:
      break;
    }
    enet_packet_destroy(event.packet);
  }
}

void GameClient::SendPacket(GamePacket &payload) {
  NET_TRACE("GameClient sending packet of type {}", payload.type);
  auto packet = enet_packet_create(&payload, payload.GetTotalSize(), 0);

  enet_peer_send(netPeer, 0, packet);
}

void GameClient::SendPacket(GamePacket &&payload) {
  SendPacket(static_cast<GamePacket &>(payload));
}
