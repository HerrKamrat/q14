#pragma once

#include <box2d/box2d.h>
#include <memory>
#include <vector>

#include "lib.hpp"

class Box2dWorld;
class GameObject;

class GameWorld : public World {
  public:
    GameWorld();
    virtual ~GameWorld();

    void init(UpdateContext& updateContext, RenderContext& renderContext) override;
    void resize(Size size) override;
    void update(UpdateContext& context) override;
    void render(RenderContext& context) override;
    bool isAnimating() const override {
        return true;
    }

  private:
    std::unique_ptr<Box2dWorld> m_physicsWorld;
    // TODO: replace with Camera-object
    Transform m_cameraTransform;
    std::vector<GameObject> m_gameObjects;
};