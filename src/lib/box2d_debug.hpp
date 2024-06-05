#pragma once

#include <box2d/box2d.h>

#include "gfx.hpp"

class Box2dDebugDraw {
  public:
    void render(b2WorldId worldId, RenderContext& context);
};
