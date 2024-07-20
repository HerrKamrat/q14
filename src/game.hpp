#pragma once

#include <box2d/box2d.h>

#include "lib.hpp"

namespace Deprecated {
class PhysicsWorld : public OldWorldImpl {
  public:
    virtual void init(UpdateContext& updateContext, RenderContext& renderContext) override;
    virtual void update(UpdateContext& context) override;
    virtual void render(RenderContext& context) override;

    virtual void onResizeEvent(ResizeEvent& resize) override;
    virtual void onKeyboardEvent(KeyboardEvent& event) override;
    virtual void resize(Size size) override;
};

}