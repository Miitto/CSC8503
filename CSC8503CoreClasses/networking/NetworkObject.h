#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"
#include "logging/logger.h"

namespace NCL::CSC8503 {
class GameObject;

#pragma region Packets
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
      : GamePacket(BasicNetworkMessages::None, sizeof(ClientPacket)) {}
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
#pragma endregion

class NetworkObject {
public:
  NetworkObject(GameObject &o, int id);
  virtual ~NetworkObject();

  // Called by clients
  virtual bool ReadPacket(GamePacket &p);
  // Called by servers
  virtual bool WritePacket(GamePacket **p, bool deltaFrame, int stateID);

  void UpdateStateHistory(int minID);

protected:
  NetworkState &GetLatestNetworkState();

  bool GetNetworkState(int frameID, NetworkState &state);

  virtual bool ReadDeltaPacket(DeltaPacket &p);
  virtual bool ReadFullPacket(FullPacket &p);

  virtual bool WriteDeltaPacket(GamePacket **p, int stateID);
  virtual bool WriteFullPacket(GamePacket **p);

  GameObject &object;

  NetworkState lastFullState;

  std::vector<NetworkState> stateHistory;

  int deltaErrors;
  int fullErrors;

  int networkID;
};
} // namespace NCL::CSC8503