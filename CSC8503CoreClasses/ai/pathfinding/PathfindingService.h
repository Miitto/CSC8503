#pragma once

#include "Channel.h"
#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "Result.h"
#include "Vector.h"
#include <future>
#include <spdlog/fmt/bundled/format.h>
#include <thread>
#include <variant>
#include <vector>

namespace NCL::CSC8503 {

class PathfindingService {
public:
  enum class PathfindingError {
    NoGridFound,
    NoPathFound,
  };

  using Result = NCL::Result<NavigationPath, PathfindingError>;

  struct ShutdownRequest {};
  struct ClearRequest {};

  struct PathfindingRequest {

    Maths::Vector3 startPos;
    Maths::Vector3 endPos;
    bool center = false;

    std::promise<PathfindingService::Result> responsePromise;
  };

  using Request = std::variant<PathfindingRequest, ShutdownRequest,
                               ClearRequest, NavigationGrid, NavigationMesh>;

  PathfindingService();
  ~PathfindingService();

  void add(NavigationGrid &&grid) { requests.send(std::move(grid)); }
  void add(NavigationMesh &&path) { requests.send(std::move(path)); }
  void clear() { requests.send(ClearRequest{}); }

  std::future<Result> requestPath(const Maths::Vector3 &from,
                                  const Maths::Vector3 &to,
                                  bool center = false);

protected:
  std::thread serverThread;
  Channel<Request> requests;
};
} // namespace NCL::CSC8503

template <>
struct fmt::formatter<NCL::CSC8503::PathfindingService::PathfindingError>
    : formatter<std::string_view> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const NCL::CSC8503::PathfindingService::PathfindingError &type,
              FormatContext &ctx) const {
    const char *typeStr = "Unknown";
    switch (type) {
    case NCL::CSC8503::PathfindingService::PathfindingError::NoGridFound:
      typeStr = "NoGridFound";
      break;
    case NCL::CSC8503::PathfindingService::PathfindingError::NoPathFound:
      typeStr = "NoPathFound";
      break;
    }
    return fmt::formatter<std::string_view>::format(typeStr, ctx);
  }
};