#include "GameServer.h"

#include "GameWorld.h"

#include "./enet/enet.h"
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
    std::cerr << "Failed to create ENet server host!" << std::endl;
    return false;
  }
  return true;
}

bool GameServer::SendGlobalPacket(int msgID) {
  GamePacket packet;
  packet.type = msgID;
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
      std::clog << "Client " << peer << " connected!" << std::endl;
      clientCount++;
      break;

    case ENET_EVENT_TYPE_DISCONNECT:
      std::clog << "Client " << peer << " disconnected!" << std::endl;
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