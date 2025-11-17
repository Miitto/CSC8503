#pragma once
#include "CollisionDetection.h"
#include "Debug.h"

namespace NCL {
using namespace NCL::Maths;
namespace CSC8503 {
template <class T> class QuadTree;

template <class T> struct QuadTreeEntry {
  Vector3 pos;
  Vector3 size;
  T object;

  QuadTreeEntry(T obj, Vector3 pos, Vector3 size) {
    object = obj;
    this->pos = pos;
    this->size = size;
  }
};

template <class T> class QuadTreeNode {
public:
  typedef std::function<void(std::list<QuadTreeEntry<T>> &)> QuadTreeFunc;

protected:
  friend class QuadTree<T>;

  using Self = QuadTreeNode<T>;

  QuadTreeNode() {}

  QuadTreeNode(Vector2 pos, Vector2 size) {
    children = nullptr;
    this->position = pos;
    this->size = size;
  }

  ~QuadTreeNode() { delete[] children; }

  void Insert(T &object, const Vector3 &objectPos, const Vector3 &objectSize,
              int depthLeft, int maxSize) {
    if (!CollisionDetection::AABBTest(
            objectPos, Vector3(position.x, 0, position.y), objectSize,
            Vector3(size.x, 1000.f, size.y)))
      return;

    if (children != nullptr) {
      for (int i = 0; i < 4; ++i) {
        children[i].Insert(object, objectPos, objectSize, depthLeft - 1,
                           maxSize);
      }

      return;
    }

    contents.emplace_back(object, objectPos, objectSize);

    if (static_cast<int>(contents.size()) > maxSize && depthLeft > 0) {
      Split();
      for (auto &i : contents) {
        for (int j = 0; j < 4; ++j) {
          children[j].Insert(i.object, i.pos, i.size, depthLeft - 1, maxSize);
        }
      }
      contents.clear();
    }
  }

  void Split() {
#ifndef NDEBUG
    if (children != nullptr) {
      std::cerr << "QuadTreeNode::Split called on already split node!"
                << std::endl;
    }
#endif

    auto halfSize = size * 0.5f;

    children = new Self[4]{
        Self(position + Vector2(-halfSize.x, halfSize.y), halfSize),
        Self(position + halfSize, halfSize),
        Self(position + Vector2(-halfSize.x, -halfSize.y), halfSize),
        Self(position + Vector2(halfSize.x, -halfSize.y), halfSize)};
  }

  void DebugDraw() {}

  void OperateOnContents(QuadTreeFunc &func) {
    if (children != nullptr) {
      for (int i = 0; i < 4; ++i) {
        children[i].OperateOnContents(func);
      }
    } else {
      if (!contents.empty())
        func(contents);
    }
  }

protected:
  std::list<QuadTreeEntry<T>> contents;

  Vector2 position;
  Vector2 size;

  QuadTreeNode<T> *children;
};

template <class T> class QuadTree {
public:
  QuadTree(Vector2 size, int maxDepth = 6, int maxSize = 5) {
    root = QuadTreeNode<T>(Vector2(), size);
    this->maxDepth = maxDepth;
    this->maxSize = maxSize;
  }
  ~QuadTree() = default;

  void Insert(T object, const Vector3 &pos, const Vector3 &size) {
    root.Insert(object, pos, size, maxDepth, maxSize);
  }

  void DebugDraw() { root.DebugDraw(); }

  void OperateOnContents(typename QuadTreeNode<T>::QuadTreeFunc func) {
    root.OperateOnContents(func);
  }

protected:
  QuadTreeNode<T> root;
  int maxDepth;
  int maxSize;
};
} // namespace CSC8503
} // namespace NCL