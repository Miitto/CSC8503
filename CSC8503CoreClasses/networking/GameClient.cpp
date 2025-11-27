#include "GameClient.h"

#include "enet/enet.h"

using namespace NCL;
using namespace CSC8503;

GameClient::GameClient() { netHandle = enet_host_create(nullptr, 1, 1, 0, 0); }

GameClient::~GameClient() { enet_host_destroy(netHandle); }

bool GameClient::Connect(IP ip) {
  ENetAddress addr{
      .host = ip.getAddress(),
      .port = ip.port(),
  };

  netPeer = enet_host_connect(netHandle, &addr, 2, 0);

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
      std::clog << "Connected to server!" << std::endl;
      break;
    case ENET_EVENT_TYPE_RECEIVE: {
      GamePacket *packet = reinterpret_cast<GamePacket *>(event.packet->data);
      ProcessPacket(packet, 0);
      break;
    }
    case ENET_EVENT_TYPE_DISCONNECT:
      // Disconnection
      break;
    default:
      break;
    }
    enet_packet_destroy(event.packet);
  }
}

void GameClient::SendPacket(GamePacket &payload) {
  auto packet = enet_packet_create(&payload, payload.GetTotalSize(), 0);

  enet_peer_send(netPeer, 0, packet);
}

void GameClient::SendPacket(GamePacket &&payload) {
  SendPacket(static_cast<GamePacket &>(payload));
}
