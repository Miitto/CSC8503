#include "Enemy.h"

namespace NCL::CSC8503 {
Enemy::Enemy(const GameWorld &w, const std::string &name)
    : GameObject(name), world(w) {}
} // namespace NCL::CSC8503