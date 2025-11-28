#pragma once
// #include "./enet/enet.h"
struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

#include <spdlog/fmt/bundled/format.h>

enum class BasicNetworkMessages : uint16_t {
  None,
  Hello,
  Message,
  String_Message,
  Delta_State,    // 1 byte per channel since the last state
  Full_State,     // Full transform etc
  Received_State, // received from a client, informs that its received packet n
  Player_Connected,
  Player_Disconnected,
  Shutdown,
  BUILTIN_MAX
};

struct GamePacketType {
  uint16_t type;

  GamePacketType(
      uint16_t type = static_cast<uint16_t>(BasicNetworkMessages::None))
      : type(type) {}
  GamePacketType(BasicNetworkMessages type)
      : type(static_cast<uint16_t>(type)) {}

  operator uint16_t() const { return type; }
  template <typename T> bool operator==(const T &b) const {
    return type == static_cast<uint16_t>(b);
  }
  template <typename T> auto operator<=>(const T &b) const {
    return type <=> static_cast<uint16_t>(b);
  }
};

template <>
struct fmt::formatter<GamePacketType> : fmt::formatter<std::string> {
  template <typename FormatContext>
  auto format(const GamePacketType &msgType, FormatContext &ctx) const {
    std::string typeName;
    switch (msgType.type) {
    case static_cast<uint16_t>(BasicNetworkMessages::None):
      typeName = "None";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Hello):
      typeName = "Hello";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Message):
      typeName = "Message";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::String_Message):
      typeName = "String_Message";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Delta_State):
      typeName = "Delta_State";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Full_State):
      typeName = "Full_State";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Received_State):
      typeName = "Received_State";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Player_Connected):
      typeName = "Player_Connected";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Player_Disconnected):
      typeName = "Player_Disconnected";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Shutdown):
      typeName = "Shutdown";
      break;
    default:
      typeName = fmt::format("Custom: {}", msgType.type);
      break;
    }
    return fmt::formatter<std::string>::format(typeName, ctx);
  }
};

struct GamePacket {
  uint16_t size = 0;
  GamePacketType type = GamePacketType(BasicNetworkMessages::None);

  GamePacket(GamePacketType type = GamePacketType(BasicNetworkMessages::None),
             uint16_t size = 0)
      : type(type), size(size) {}
  int GetTotalSize() { return sizeof(GamePacket) + size; }

  template <typename T>
    requires std::is_base_of<GamePacket, T>::value
  static T *as(GamePacket *base) {
    return reinterpret_cast<T *>(base);
  }
};

class PacketReceiver {
public:
  virtual void ReceivePacket(GamePacketType type, GamePacket *payload,
                             int source = -1) = 0;
};

class NetworkBase {
public:
  static void Initialise();
  static void Destroy();

  constexpr inline static uint16_t GetDefaultPort() { return 1234; }

  void RegisterPacketHandler(GamePacketType type, PacketReceiver *receiver) {
    packetHandlers.insert(std::make_pair(type, receiver));
  }

protected:
  NetworkBase();
  ~NetworkBase();

  bool ProcessPacket(GamePacket *p, int peerID = -1);

  typedef std::multimap<GamePacketType, PacketReceiver *>::const_iterator
      PacketHandlerIterator;

  struct IteratorRange {
    PacketHandlerIterator first;
    PacketHandlerIterator last;

    PacketHandlerIterator begin() const { return first; }
    PacketHandlerIterator end() const { return last; }
  };

  std::optional<IteratorRange> GetPacketHandlers(int msgID) const {
    auto range = packetHandlers.equal_range(msgID);

    if (range.first == packetHandlers.end()) {
      return std::nullopt; // no handlers for this message type!
    }
    return IteratorRange{.first = range.first, .last = range.second};
  }

  _ENetHost *netHandle;

  std::multimap<GamePacketType, PacketReceiver *> packetHandlers;
};