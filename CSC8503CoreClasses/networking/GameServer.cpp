#include "GameServer.h"

#include "GameWorld.h"

#include "./enet/enet.h"

#include "logging/logger.h"

using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(uint16_t onPort, int maxClients) {
  port = onPort;
  clientMax = maxClients;
  clientCount = 0;
  netHandle = nullptr;
  Initialise();
}

GameServer::~GameServer() { Shutdown(); }

void GameServer::Shutdown() {
  SendGlobalPacket(BasicNetworkMessages::Shutdown);
  enet_host_destroy(netHandle);
  netHandle = nullptr;
}

bool GameServer::Initialise() {
  ENetAddress address{
      .host = ENET_HOST_ANY,
      .port = port,
  };

  netHandle = enet_host_create(&address, clientMax, 1, 0, 0);

  if (!netHandle) {
    NET_ERROR("Failed to create ENet server host!");
    return false;
  }
  return true;
}

ENetPeer *GameServer::GetPeer(int id) {
  for (size_t i = 0; i < netHandle->peerCount; i++) {
    ENetPeer *p = &netHandle->peers[i];
    if (p->incomingPeerID == static_cast<enet_uint16>(id)) {
      return p;
    }
  }
  return nullptr;
}

bool GameServer::SendPacketToClient(int clientID, GamePacketType type) {
  GamePacket packet;
  packet.type = type;
  return SendPacketToClient(clientID, packet);
}

bool GameServer::SendPacketToClient(int clientID, GamePacket &packet) {
  ENetPacket *enetPacket =
      enet_packet_create(&packet, packet.GetTotalSize(), 0);

  auto peer = GetPeer(clientID);

  if (!peer) {
    NET_ERROR("No such client with ID {}", clientID);
    enet_packet_destroy(enetPacket);
    return false;
  }
  enet_peer_send(peer, 0, enetPacket);
  return true;
}

bool GameServer::SendPacketToClient(int clientID, GamePacket &&packet) {
  return SendPacketToClient(clientID, static_cast<GamePacket &>(packet));
}

bool GameServer::SendGlobalPacket(GamePacketType type) {
  GamePacket packet;
  packet.type = type;
  return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket &packet) {
  ENetPacket *enetPacket =
      enet_packet_create(&packet, packet.GetTotalSize(), 0);

  enet_host_broadcast(netHandle, 0, enetPacket);
  return true;
}

bool GameServer::SendGlobalPacket(GamePacket &&packet) {
  return SendGlobalPacket(static_cast<GamePacket &>(packet));
}

void GameServer::UpdateServer() {
  if (!netHandle)
    return;

  ENetEvent event;
  while (enet_host_service(netHandle, &event, 0) > 0) {
    int type = event.type;
    ENetPeer *p = event.peer;
    int peer = p->incomingPeerID;

    switch (type) {
    case ENET_EVENT_TYPE_CONNECT:
      NET_LOG("Client {} connected.", peer);
      clientCount++;
      break;

    case ENET_EVENT_TYPE_DISCONNECT:
      NET_LOG("Client {} disconnected.", peer);
      --clientCount;
      break;
    case ENET_EVENT_TYPE_RECEIVE: {
      GamePacket *packet = reinterpret_cast<GamePacket *>(event.packet->data);
      ProcessPacket(packet, peer);
    }
    }
    enet_packet_destroy(event.packet);
  }
}

void GameServer::SetGameWorld(GameWorld &g) { gameWorld = &g; }