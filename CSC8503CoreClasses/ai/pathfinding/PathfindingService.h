#pragma once

#include "Channel.h"
#include "Vector.h"
#include <mutex>
#include <thread>
#include <variant>
#include <vector>

namespace NCL::CSC8503 {

class PathfindingServiceRequest {
public:
  PathfindingServiceRequest() {}

  bool isComplete() const {
    // Doesn't change any state, but we do need to lock the mutex to read 'done'
    auto &mutex = const_cast<std::mutex &>(this->mutex);
    auto lock = mutex.try_lock();
    if (!lock)
      return false;

    bool d = done;
    mutex.unlock();
    return d;
  }

protected:
  std::mutex mutex = {};
  bool done = false;
  std::vector<Maths::Vector3> path;
};

class PathfindingServiceRequestServer {
public:
  PathfindingServiceRequestServer(
      const Maths::Vector3 start, const Maths::Vector3 end,
      std::shared_ptr<PathfindingServiceRequest> clientReq)
      : startPos(start), endPos(end), clientRequest(clientReq) {}

protected:
  Maths::Vector3 startPos;
  Maths::Vector3 endPos;

  std::shared_ptr<PathfindingServiceRequest> clientRequest;
};

class PathfindingServiceServer {
public:
  struct ShutdownRequest {};
  using Request =
      std::variant<PathfindingServiceRequestServer, ShutdownRequest>;

  PathfindingServiceServer(Channel<Request> channel) : requests(channel) {}

  void run();
  void handleRequest(const PathfindingServiceRequestServer &request);

protected:
  Channel<Request> requests;
};

class PathfindingService {
public:
  PathfindingService();

protected:
  std::thread serverThread;
  Channel<PathfindingServiceServer::Request> requests;
};
} // namespace NCL::CSC8503