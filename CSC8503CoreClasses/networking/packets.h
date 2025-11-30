#pragma once

#include "networking/NetworkBase.h"
#include "networking/NetworkState.h"

namespace NCL::CSC8503 {

struct PlayerConnectedPacket : public GamePacket {
  int id = -1;

  PlayerConnectedPacket(int id)
      : GamePacket(BasicNetworkMessages::Player_Connected,
                   sizeof(PlayerConnectedPacket) - sizeof(GamePacket)),
        id(id) {}
};

struct PlayerDisconnectedPacket : public GamePacket {
  int id = -1;

  PlayerDisconnectedPacket(int id)
      : GamePacket(BasicNetworkMessages::Player_Connected,
                   sizeof(PlayerConnectedPacket) - sizeof(GamePacket)),
        id(id) {}
};

struct FullPacket : public GamePacket {
  int objectID = -1;
  NetworkState fullState;

  FullPacket()
      : GamePacket(BasicNetworkMessages::Full_State,
                   sizeof(FullPacket) - sizeof(GamePacket)) {}
};

struct DeltaPacket : public GamePacket {
  int fullID = -1;
  int objectID = -1;
  char pos[3] = {0, 0, 0};
  char orientation[4] = {0, 0, 0, 1};

  DeltaPacket()
      : GamePacket(BasicNetworkMessages::Delta_State,
                   sizeof(DeltaPacket) - sizeof(GamePacket)) {}
};

struct ClientPacket : public GamePacket {
  int lastID = 0;
  char buttonstates[8] = {};

  ClientPacket()
      : GamePacket(BasicNetworkMessages::PlayerState,
                   sizeof(ClientPacket) - sizeof(GamePacket)) {}
};

struct AckPacket : public GamePacket {
  int receivedID;
  AckPacket() = delete;
  AckPacket(int received)
      : GamePacket(BasicNetworkMessages::Received_State,
                   sizeof(AckPacket) - sizeof(GamePacket)),
        receivedID(received) {}
};

struct StringPacket : public GamePacket {
  char data[256];
  StringPacket(std::string_view str)
      : GamePacket(BasicNetworkMessages::String_Message,
                   static_cast<uint16_t>(str.length())) {

    NET_ASSERT(str.length() <= 256,
               "String has too many characters for a single packet");

    memcpy(data, str.data(), str.length());
  }

  std::string_view view() const { return std::string_view(data, size); }
  std::string get() const { return std::string(data, size); }
  operator std::string() const { return get(); }
  operator std::string_view() const { return view(); }
};

struct LevelChangePacket : public GamePacket {
  uint8_t level;
  LevelChangePacket(uint8_t level)
      : GamePacket(BasicNetworkMessages::LevelChange,
                   sizeof(LevelChangePacket) - sizeof(GamePacket)),
        level(level) {}
};
} // namespace NCL::CSC8503
