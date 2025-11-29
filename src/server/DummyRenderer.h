#pragma once

#include "GameTechRendererInterface.h"

class DummyRenderer : public NCL::CSC8503::GameTechRendererInterface {
public:
  DummyRenderer() {}
  NCL::Rendering::Mesh *LoadMesh(const std::string &name) override {
    return nullptr;
  }
  NCL::Rendering::Texture *LoadTexture(const std::string &name) override {
    return nullptr;
  }
};