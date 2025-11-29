#pragma once
// #include "./enet/enet.h"
struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

#include <spdlog/fmt/bundled/format.h>

enum class BasicNetworkMessages : uint16_t {
  /// @brief Packet sent client->server when first connecting
  Hello,
  /// @brief Quick 0-size packet to check latency / connection
  Ping,
  /// @brief Response to a ping
  Ping_Response,
  /// @brief Generic message with a short ID
  Message,
  /// @brief Generic message with a string
  String_Message,
  /// @brief Delta state update sent server->client
  Delta_State, // 1 byte per channel since the last state
  /// @brief Full state update sent server->client
  Full_State, // Full transform etc
  /// @brief Player state update sent client->server
  PlayerState,
  /// @brief Server acknowledgment of a received player state
  Received_State, // received from a client, informs that its received packet n
  /// @brief Sent server->clients when a new player connects
  Player_Connected,
  /// @brief Sent server->clients when a player disconnects
  Player_Disconnected,
  /// @brief Sent client->server when a client is disconnecting, or
  /// server->clients when server shutting down
  Shutdown,
  /// @brief Max built-in message ID for custom messages to start from
  BUILTIN_MAX
};

struct GamePacketType {
  uint16_t type;

  GamePacketType(
      uint16_t type = static_cast<uint16_t>(BasicNetworkMessages::PlayerState))
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
    case static_cast<uint16_t>(BasicNetworkMessages::PlayerState):
      typeName = "PlayerState";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Hello):
      typeName = "Hello";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Ping):
      typeName = "Ping";
      break;
    case static_cast<uint16_t>(BasicNetworkMessages::Ping_Response):
      typeName = "Ping_Response";
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
  GamePacketType type = GamePacketType(BasicNetworkMessages::PlayerState);

  GamePacket(
      GamePacketType type = GamePacketType(BasicNetworkMessages::PlayerState),
      uint16_t size = 0)
      : type(type), size(size) {}
  int GetTotalSize() { return sizeof(GamePacket) + size; }

  template <typename T>
    requires std::is_base_of<GamePacket, T>::value
  static T *as(GamePacket *base) {
    return reinterpret_cast<T *>(base);
  }

  template <typename T>
    requires std::is_base_of<GamePacket, T>::value
  static T &as(GamePacket &base) {
    return (reinterpret_cast<T &>(base));
  }

  template <typename T> static T as(const GamePacket &base) {
    static_assert(std::is_base_of<GamePacket, T>::value,
                  "GamePacket::as<T> can only be used with types derived from "
                  "GamePacket");
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

  _ENetHost *netHandle = nullptr;

  std::multimap<GamePacketType, PacketReceiver *> packetHandlers;
};