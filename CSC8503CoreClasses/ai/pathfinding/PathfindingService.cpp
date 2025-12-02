#include "PathfindingService.h"

namespace NCL::CSC8503 {
PathfindingService::PathfindingService() {
  Channel<PathfindingServiceServer::Request> channel;

  serverThread = std::thread([this, chan = channel]() mutable {
    PathfindingServiceServer server(chan);
    server.run();
  });
}

PathfindingService::~PathfindingService() {
  requests.send(PathfindingServiceServer::ShutdownRequest{});
  serverThread.join();
}

void PathfindingServiceServer::run() {
  bool running = true;
  while (running) {
    auto reqOpt = requests.receive();
    if (!reqOpt.has_value()) {
      // No requests available, sleep briefly to avoid busy-waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    auto &req = reqOpt.value();
    std::visit(
        [&](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, ShutdownRequest>) {
            running = false;
          } else if constexpr (std::is_same_v<
                                   T, PathfindingServiceRequestServer>) {
            handleRequest(arg);
          }
        },
        req);
  }
}

void PathfindingServiceServer::handleRequest(
    const PathfindingServiceRequestServer &request) {
  // TODO: Implement pathfinding logic here
}
} // namespace NCL::CSC8503