#pragma once

#include "Keyboard.h"
#include "Mouse.h"
#include "Window.h"

class DummyKeyboard : public NCL::Keyboard {};

class DummyMouse : public NCL::Mouse {};

class DummyWindow : public NCL::Window {
public:
  DummyWindow() {
    init = true;
    size = NCL::Vector2i(800, 600);
    position = NCL::Vector2i(100, 100);
    windowTitle = "Dummy Window";

    keyboard = new DummyKeyboard();
    mouse = new DummyMouse();
  }

  bool InternalUpdate() override { return true; }
};