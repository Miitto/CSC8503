#include "NavigationGrid.h"
#include "Assets.h"

#include <fstream>

using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE = 0;
const int RIGHT_NODE = 1;
const int TOP_NODE = 2;
const int BOTTOM_NODE = 3;

const char WALL_NODE = 'x';
const char FLOOR_NODE = '.';

NavigationGrid::NavigationGrid(const std::string &filename,
                               const Vector3 origin)
    : NavigationGrid() {
  gridOrigin = origin;

  std::ifstream infile(Assets::DATADIR + filename);

  infile >> nodeSize;
  infile >> gridWidth;
  infile >> gridHeight;

  allNodes = new GridNode[gridWidth * gridHeight];

  for (int y = 0; y < gridHeight; ++y) {
    for (int x = 0; x < gridWidth; ++x) {
      GridNode &n = allNodes[(gridWidth * y) + x];
      char type = 0;
      infile >> type;
      n.type = type;
      n.position = Vector3((float)(x * nodeSize), 0, (float)(y * nodeSize));
    }
  }

  // now to build the connectivity between the nodes
  for (int y = 0; y < gridHeight; ++y) {
    for (int x = 0; x < gridWidth; ++x) {
      GridNode &n = allNodes[(gridWidth * y) + x];

      if (y > 0) { // get the above node
        n.connections[0].node = &allNodes[(gridWidth * (y - 1)) + x];
      }
      if (y < gridHeight - 1) { // get the below node
        n.connections[1].node = &allNodes[(gridWidth * (y + 1)) + x];
      }
      if (x > 0) { // get left node
        n.connections[2].node = &allNodes[(gridWidth * (y)) + (x - 1)];
      }
      if (x < gridWidth - 1) { // get right node
        n.connections[3].node = &allNodes[(gridWidth * (y)) + (x + 1)];
      }
      for (int i = 0; i < 4; ++i) {
        if (n.connections[i].node) {
          if (n.connections[i].node->type == '.') {
            n.connections[i].cost = 1;
          }
          if (n.connections[i].node->type == 'x') {
            n.connections[i].node = nullptr; // actually a wall, disconnect!
          }
        }
      }
    }
  }
}

NavigationGrid::~NavigationGrid() {
  if (allNodes)
    delete[] allNodes;
}

NavigationGrid::NavigationGrid(NavigationGrid &&other) noexcept
    : nodeSize(other.nodeSize), gridWidth(other.gridWidth),
      gridHeight(other.gridHeight), gridOrigin(other.gridOrigin),
      allNodes(other.allNodes) {
  other.allNodes = nullptr;
}

bool NavigationGrid::containsPoint(const Vector3 &point) const {
  float xMin = gridOrigin.x;
  float xMax = gridOrigin.x + (gridWidth * nodeSize);
  float zMin = gridOrigin.z;
  float zMax = gridOrigin.z + (gridHeight * nodeSize);

  return (point.x >= xMin && point.x <= xMax && point.z >= zMin &&
          point.z <= zMax);
}

bool NavigationGrid::FindPath(const Vector3 &fromWorld, const Vector3 &toWorld,
                              NavigationPath &outPath, bool centered) {
  // need to work out which node 'from' sits in, and 'to' sits in
  Vector3 from = fromWorld - gridOrigin;
  Vector3 to = toWorld - gridOrigin;
  int fromX = ((int)from.x / nodeSize);
  int fromZ = ((int)from.z / nodeSize);

  int toX = ((int)to.x / nodeSize);
  int toZ = ((int)to.z / nodeSize);

  if (fromX < 0 || fromX > gridWidth - 1 || fromZ < 0 ||
      fromZ > gridHeight - 1) {
    return false; // outside of map region!
  }

  if (toX < 0 || toX > gridWidth - 1 || toZ < 0 || toZ > gridHeight - 1) {
    return false; // outside of map region!
  }

  GridNode *startNode = &allNodes[(fromZ * gridWidth) + fromX];
  GridNode *endNode = &allNodes[(toZ * gridWidth) + toX];

  std::vector<GridNode *> openList;
  std::vector<GridNode *> closedList;

  openList.push_back(startNode);

  startNode->f = 0;
  startNode->g = 0;
  startNode->parent = nullptr;

  GridNode *currentBestNode = nullptr;

  while (!openList.empty()) {
    currentBestNode = RemoveBestNode(openList);

    if (currentBestNode == endNode) { // we've found the path!
      GridNode *node = endNode;
      while (node != nullptr) {
        Vector3 pos = node->position + gridOrigin;
        if (centered) {
          pos.x += nodeSize / 2.0f;
          pos.z += nodeSize / 2.0f;
        }
        outPath.PushWaypoint(pos);
        node = node->parent;
      }
      return true;
    } else {
      for (int i = 0; i < 4; ++i) {
        GridNode *neighbour = currentBestNode->connections[i].node;
        if (!neighbour) { // might not be connected...
          continue;
        }
        bool inClosed = NodeInList(neighbour, closedList);
        if (inClosed) {
          continue; // already discarded this neighbour...
        }

        float h = Heuristic(neighbour, endNode);
        float g = currentBestNode->g + currentBestNode->connections[i].cost;
        float f = h + g;

        bool inOpen = NodeInList(neighbour, openList);

        if (!inOpen) { // first time we've seen this neighbour
          openList.emplace_back(neighbour);
        }
        if (!inOpen ||
            f < neighbour->f) { // might be a better route to this neighbour
          neighbour->parent = currentBestNode;
          neighbour->f = f;
          neighbour->g = g;
        }
      }
      closedList.emplace_back(currentBestNode);
    }
  }
  return false; // open list emptied out with no path!
}

bool NavigationGrid::NodeInList(GridNode *n,
                                std::vector<GridNode *> &list) const {
  return std::find(list.begin(), list.end(), n) != list.end();
}

GridNode *NavigationGrid::RemoveBestNode(std::vector<GridNode *> &list) const {
  std::vector<GridNode *>::iterator bestI = list.begin();

  GridNode *bestNode = *list.begin();

  for (auto i = list.begin(); i != list.end(); ++i) {
    if ((*i)->f < bestNode->f) {
      bestNode = (*i);
      bestI = i;
    }
  }
  list.erase(bestI);

  return bestNode;
}

float NavigationGrid::Heuristic(GridNode *hNode, GridNode *endNode) const {
  return Vector::Length(hNode->position - endNode->position);
}

bool NavigationGrid::isNodeWalkable(int x, int y) const {
  if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
    return false;
  }
  GridNode &n = allNodes[(gridWidth * y) + x];
  return n.type == FLOOR_NODE;
}