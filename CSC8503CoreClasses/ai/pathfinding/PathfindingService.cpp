#include "PathfindingService.h"
#include "Overloaded.h"
#include "VectorFormat.h"
#include "logging/logger.h"

namespace NCL::CSC8503 {

class PathfindingServer {
public:
  PathfindingServer(mpsc::Receiver<PathfindingService::ServerRequest> &&channel)
      : requests(std::move(channel)) {
    AI_DEBUG("Starting Pathfinding Server");
  }
  ~PathfindingServer() { AI_DEBUG("Shutting down Pathfinding Server"); }

  void run() {
    bool running = true;
    while (running) {
      AI_DEBUG("Pathfinding Server Receiving");
      auto reqRes = requests.receive();
      if (reqRes.is_err()) {
        AI_DEBUG("Pathfinding Server Channel Closed, Shutting Down");
        break;
      }

      auto &req = reqRes.unwrap_ref();

      AI_DEBUG("Pathfinding Server Received ServerRequest");
      std::visit(NCL::overloaded{
                     [&](const PathfindingService::ShutdownRequest &) {
                       AI_DEBUG("Shutting down Pathfinding Server");
                       running = false;
                     },
                     [&](const PathfindingService::ClearRequest &) {
                       AI_DEBUG("Clearing all navigation data");
                       grids.clear();
                       paths.clear();
                     },
                     [&](NavigationGrid &grid) {
                       addGrid(std::move(grid));
                       AI_DEBUG("Added grid");
                     },
                     [&](NavigationMesh &mesh) {
                       addPath(std::move(mesh));
                       AI_DEBUG("Added mesh");
                     },
                     [&](PathfindingService::PathfindingRequest &request) {
                       AI_DEBUG("Handling pathfinding request");
                       handleRequest(request);
                     },
                 },
                 req);
    }
  }

protected:
  void addGrid(NavigationGrid &&grid) {
    NavigationGrid::MinMax bounds = grid.GetGridBounds();

    AI_DEBUG("Adding Navigation Grid: Width {}, Height {}, Node Size "
             "{} at {}\nExtents: ({}, {}) -> ({}, {})",
             grid.GetWidth(), grid.GetHeight(), grid.GetNodeSize(),
             grid.GetOrigin(), bounds.min.x, bounds.min.y, bounds.max.x,
             bounds.max.y);

    grids.emplace_back(std::move(grid));
  }
  void addPath(NavigationMesh &&mesh) { paths.emplace_back(std::move(mesh)); }
  void handleRequest(PathfindingService::PathfindingRequest &request) {
    // TODO: Implement pathfinding logic here

    auto navDataOpt = findSuitableNav(request.startPos, request.endPos);
    if (!navDataOpt.has_value()) {
      request.responsePromise.set_value(PathfindingService::Result::err(
          PathfindingService::PathfindingError::NoGridFound));
      return;
    }

    std::visit(NCL::overloaded{
                   [&](NavigationGrid *grid) {
                     GetPath(*grid, request.startPos, request.endPos, request);
                   },
                   [&](NavigationMesh *mesh) {
                     GetPath(*mesh, request.startPos, request.endPos, request);
                   },
               },
               *navDataOpt);
  }

  using NavData = std::variant<NavigationGrid *, NavigationMesh *>;

  std::optional<NavData> findSuitableNav(const Maths::Vector3 &from,
                                         const Maths::Vector3 &to) {
    for (auto &grid : grids) {
      if (grid.containsPoint(to) && grid.containsPoint(from))
        return &grid;
    }

    for (auto &mesh : paths) {
      if (mesh.containsPoint(to) && mesh.containsPoint(from))
        return &mesh;
    }

    return std::nullopt;
  }

  void GetPath(NavigationGrid &grid, const Maths::Vector3 &from,
               const Maths::Vector3 &to,
               PathfindingService::PathfindingRequest &request) {
    NavigationPath path;
    if (grid.FindPath(from, to, path, request.center)) {
      request.responsePromise.set_value(std::move(path));
    } else {
      request.responsePromise.set_value(PathfindingService::Result::err(
          PathfindingService::PathfindingError::NoPathFound));
    }
  }

  void GetPath(NavigationMesh &mesh, const Maths::Vector3 &from,
               const Maths::Vector3 &to,
               PathfindingService::PathfindingRequest &request) {
    NavigationPath path;
    if (mesh.FindPath(from, to, path)) {
      request.responsePromise.set_value(std::move(path));
    } else {
      request.responsePromise.set_value(PathfindingService::Result::err(
          PathfindingService::PathfindingError::NoPathFound));
    }
  }

  mpsc::Receiver<PathfindingService::ServerRequest> requests;
  std::vector<NavigationGrid> grids;
  std::vector<NavigationMesh> paths;
};

PathfindingService::PathfindingService() {
  auto [sender, receiver] = mpsc::create<PathfindingService::ServerRequest>();
  requests = sender;

  serverThread = std::thread([r = std::move(receiver)]() mutable {
    PathfindingServer server(std::move(r));
    server.run();
  });
}

PathfindingService::~PathfindingService() {
  AI_DEBUG("Sending shutdown request to Pathfinding Server");
  requests.send(PathfindingService::ShutdownRequest{});
  serverThread.join();
}

std::future<PathfindingService::Result>
PathfindingService::requestPath(const Maths::Vector3 &from,
                                const Maths::Vector3 &to, bool center) const {
  std::promise<PathfindingService::Result> promise;
  auto future = promise.get_future();

  AI_DEBUG("Requesting {}path from {} to {}", center ? "centered " : "", from,
           to);
  requests.send(PathfindingRequest{from, to, center, std::move(promise)});

  return future;
}
} // namespace NCL::CSC8503
