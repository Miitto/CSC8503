#pragma once
#include "NavigationMap.h"
#include <array>
#include <string>

namespace NCL::CSC8503 {
struct GridNode {
  GridNode *parent = nullptr;

  struct Connection {
    GridNode *node;
    int cost;
  };

  std::array<Connection, 4> connections = {{nullptr, 0}};

  Vector3 position = {};

  float f = 0;
  float g = 0;

  int type = 0;

  GridNode() = default;
  ~GridNode() = default;
};

class NavigationGrid : public NavigationMap {
public:
  NavigationGrid() = default;
  NavigationGrid(const std::string &filename);
  ~NavigationGrid();

  NavigationGrid(const NavigationGrid &other) = delete;
  NavigationGrid(NavigationGrid &&other) noexcept;

  bool FindPath(const Vector3 &from, const Vector3 &to,
                NavigationPath &outPath) override;

  bool containsPoint(const Vector3 &point) const;

protected:
  bool NodeInList(GridNode *n, std::vector<GridNode *> &list) const;
  GridNode *RemoveBestNode(std::vector<GridNode *> &list) const;
  float Heuristic(GridNode *hNode, GridNode *endNode) const;
  int nodeSize = 0;
  int gridWidth = 0;
  int gridHeight = 0;
  Vector3 gridOrigin = {};

  GridNode *allNodes = nullptr;
};
} // namespace NCL::CSC8503
