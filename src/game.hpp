#pragma once

#include "lib.hpp"

class PhysicsWorld : public World {
  public:
    virtual void init(UpdateContext& updateContext, RenderContext& renderContext) override;
    virtual void update(UpdateContext& context) override;
    virtual void render(RenderContext& context) override;

    virtual void onResizeEvent(ResizeEvent& resize) override;
    virtual void onKeyboardEvent(KeyboardEvent& event) override;
};