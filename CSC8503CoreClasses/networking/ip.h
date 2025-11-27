#pragma once

#include <string>

namespace NCL {
class IP {
public:
  constexpr inline IP(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port)
      : address((d << 24) | (c << 16) | (b << 8) | a), p(port) {}

  uint8_t a() const { return (address >> 0) & 0xFF; }
  uint8_t b() const { return (address >> 8) & 0xFF; }
  uint8_t c() const { return (address >> 16) & 0xFF; }
  uint8_t d() const { return (address >> 24) & 0xFF; }
  uint16_t port() const { return p; }
  IP &setPort(uint16_t port) {
    p = port;
    return *this;
  }

  uint32_t getAddress() const { return address; }

  operator std::string() const {
    return std::to_string(a()) + "." + std::to_string(b()) + "." +
           std::to_string(c()) + "." + std::to_string(d()) + ":" +
           std::to_string(p);
  }

protected:
  uint32_t address;
  uint16_t p;
};
} // namespace NCL