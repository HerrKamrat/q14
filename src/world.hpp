#pragma once

#include <box2d/box2d.h>
#include <memory>
#include <vector>

#include "lib.hpp"

class PhysicsSystem;
class GameObject;

struct GameContext {
    PhysicsSystem* physics;
};

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

    GameContext getContext();

  private:
    std::unique_ptr<PhysicsSystem> m_physics;
    // TODO: replace with Camera-object
    Transform m_cameraTransform;
    std::vector<GameObject> m_gameObjects;
};