#pragma once
#include "NavigationMap.h"
#include <array>
#include <string>
#include <vector>

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
  NavigationGrid(const std::string &filename, const Vector3 origin = {});

  bool FindPath(const Vector3 &from, const Vector3 &to, NavigationPath &outPath,
                bool centered = false) override;

  bool containsPoint(const Vector3 &point) const;

  int GetWidth() const { return gridWidth; }
  int GetHeight() const { return gridHeight; }
  int GetNodeSize() const { return nodeSize; }
  const Vector3 &GetOrigin() const { return gridOrigin; }

  bool isNodeWalkable(int x, int y) const;

  struct MinMax {
    Vector2 min;
    Vector2 max;
  };

  MinMax GetGridBounds() const {
    MinMax bounds;
    bounds.min = Vector2(gridOrigin.x, gridOrigin.z);
    bounds.max = Vector2(gridOrigin.x + gridWidth * nodeSize,
                         gridOrigin.z + gridHeight * nodeSize);
    return bounds;
  }

protected:
  bool NodeInList(GridNode *n, std::vector<GridNode *> &list) const;
  GridNode *RemoveBestNode(std::vector<GridNode *> &list) const;
  float Heuristic(GridNode *hNode, GridNode *endNode) const;
  int nodeSize = 0;
  int gridWidth = 0;
  int gridHeight = 0;
  Vector3 gridOrigin = {};

  std::vector<GridNode> allNodes;
};
} // namespace NCL::CSC8503
