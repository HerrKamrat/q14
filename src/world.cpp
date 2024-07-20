#include "world.hpp"
#include "lib/box2d_debug.hpp"

class Sensor {
  public:
    bool inCollision() const {
        return m_collisionCount > 0;
    }

    void onCollisionBegan() {
        SDL_Log("Sensor collision began");
        m_collisionCount++;
    };

    void onCollisionEnded() {
        SDL_Log("Sensor collision ended");
        m_collisionCount--;
    };

  private:
    int m_collisionCount = 0;
};

class Component {
  public:
    virtual ~Component() = default;
    virtual void update(UpdateContext& context) {};
    virtual void render() {};
};

class DynamicBodyComponent : public Component {
  public:
    bool onGround() const {
        return m_bottomSensor.inCollision();
    }

    bool onLeftWall() const {
        return m_leftSensor.inCollision();
    }

    bool onRightWall() const {
        return m_rightSensor.inCollision();
    }

    bool onWall() const {
        return onLeftWall() || onRightWall();
    }

    void applyForce(Vec2 force) {
        b2Body_ApplyForceToCenter(m_id, {force.x, force.y}, true);
    }

    b2BodyId m_id;
    Sensor m_leftSensor;
    Sensor m_rightSensor;
    Sensor m_topSensor;
    Sensor m_bottomSensor;
};

class InputComponent : public Component {
  public:
    InputComponent(DynamicBodyComponent* ptr)
        : m_ptr(ptr){

          };

    void update(UpdateContext& context) override {
        auto input = context.getInputState();
        if (input.left.active()) {
            m_ptr->applyForce({-10.0f, 0.0f});
        } else if (input.right.active()) {
            m_ptr->applyForce({10.0f, 0.0f});
        }
    };

    DynamicBodyComponent* m_ptr;
};

class GameObject {
  public:
    void update(UpdateContext& context) {
        for (auto& component : m_components) {
            component->update(context);
        }
    };

    void render(RenderContext& context) {
        // for (auto& component : m_components) {
        //     component->render();
        // }
    };
    void addComponent(std::unique_ptr<Component> component) {
        m_components.push_back(std::move(component));
    };

  private:
    std::vector<std::unique_ptr<Component>> m_components;
};

class Box2dWorld {
  public:
    bool valid() const {
        return m_id.index1 != b2_nullWorldId.index1 || m_id.revision != b2_nullWorldId.revision;
    }

    void init() {
        if (valid()) {
            reset();
        }
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0.0f, 10.0f};
        m_id = b2CreateWorld(&worldDef);

        const Vec2 position = Vec2{8, 15.0f + 0.5f};
        const int size = 16;

        b2BodyDef groundBodyDef = b2DefaultBodyDef();
        groundBodyDef.position = {position.x, position.y};

        b2BodyId groundId = b2CreateBody(m_id, &groundBodyDef);

        b2Polygon groundBox = b2MakeBox(0.5f * size, 0.5f);

        b2ShapeDef groundShapeDef = b2DefaultShapeDef();
        // groundShapeDef.userData = ptr;
        groundShapeDef.friction = 1.0f;
        groundShapeDef.restitution = 0.0f;
        b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
    }

    void update(UpdateContext& context) {
        int subStepCount = 4;
        b2World_Step(m_id, context.getDeltaTime(), subStepCount);

        for (auto& event : getSensorBeginTouchEvents()) {
            auto userData = b2Shape_GetUserData(event.sensorShapeId);
            if (!userData) {
                continue;
            }
            // auto visitorUserData = b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));

            auto sensor = reinterpret_cast<Sensor*>(userData);
            // auto visitor = reinterpret_cast<GameObject*>(visitorUserData);
            sensor->onCollisionBegan();
        };

        for (auto& event : getSensorBeginTouchEvents()) {
            auto userData = b2Shape_GetUserData(event.sensorShapeId);
            if (!userData) {
                continue;
            }

            // auto visitorUserData = b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));
            auto sensor = reinterpret_cast<Sensor*>(userData);
            // auto visitor = reinterpret_cast<GameObject*>(visitorUserData);
            sensor->onCollisionEnded();
        };
    };

    void render(RenderContext& context) {
        m_debugDraw.render(m_id, context);
    }

    std::unique_ptr<DynamicBodyComponent> create() {
        std::unique_ptr<DynamicBodyComponent> component = std::make_unique<DynamicBodyComponent>();
        const Vec2 position{7 + 0.5f, 14 + 0.5f};

        b2Polygon dynamicBox = b2MakeBox(0.25f, 0.5f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.friction = 1.0f;
        shapeDef.restitution = 0.0f;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.fixedRotation = true;
        bodyDef.position = {position.x, position.y};

        bodyDef.userData = &component;
        b2BodyId bodyId = b2CreateBody(m_id, &bodyDef);
        [[maybe_unused]] b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
        component->m_id = bodyId;

        {
            b2Polygon polygon = b2MakeOffsetBox(0.15f, 0.05f, {0, 0.5f}, 0);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.isSensor = true;

            shapeDef.userData = &component->m_bottomSensor;

            b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        }

        {
            b2Polygon polygon = b2MakeOffsetBox(0.15f, 0.05f, {0, -0.5f}, 0);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.isSensor = true;

            shapeDef.userData = &component->m_topSensor;

            b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        }

        {
            b2Polygon polygon = b2MakeOffsetBox(0.05f, 0.45f, {0.25f, 0}, 0);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.isSensor = true;
            shapeDef.userData = &component->m_rightSensor;
            b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        }

        {
            b2Polygon polygon = b2MakeOffsetBox(0.05f, 0.45f, {-0.25f, 0}, 0);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.isSensor = true;
            shapeDef.userData = &component->m_leftSensor;
            b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        }

        return component;
    }

    std::span<b2BodyMoveEvent> getMoveEvents() {
        b2BodyEvents events = b2World_GetBodyEvents(m_id);
        return {events.moveEvents, (size_t)events.moveCount};
    }

    std::span<b2ContactBeginTouchEvent> getBeginTouchEvents() {
        b2ContactEvents events = b2World_GetContactEvents(m_id);
        return {events.beginEvents, (size_t)events.beginCount};
    }

    std::span<b2ContactEndTouchEvent> getEndTouchEvents() {
        b2ContactEvents events = b2World_GetContactEvents(m_id);
        return {events.endEvents, (size_t)events.endCount};
    }

    std::span<b2ContactHitEvent> getHitEvents() {
        b2ContactEvents events = b2World_GetContactEvents(m_id);
        return {events.hitEvents, (size_t)events.hitCount};
    }

    std::span<b2SensorBeginTouchEvent> getSensorBeginTouchEvents() {
        b2SensorEvents events = b2World_GetSensorEvents(m_id);
        return {events.beginEvents, (size_t)events.beginCount};
    }

    std::span<b2SensorEndTouchEvent> getSensorEndTouchEvents() {
        b2SensorEvents events = b2World_GetSensorEvents(m_id);
        return {events.endEvents, (size_t)events.endCount};
    }

  private:
    void reset() {
        b2DestroyWorld(m_id);
        m_id = b2_nullWorldId;
    }

    b2WorldId m_id = b2_nullWorldId;
    Box2dDebugDraw m_debugDraw;
};

GameWorld::GameWorld() = default;

GameWorld::~GameWorld() = default;

void GameWorld::init(UpdateContext& updateContext, RenderContext& renderContext) {
    m_physicsWorld = std::make_unique<Box2dWorld>();
    m_physicsWorld->init();
    m_gameObjects.emplace_back();
    {
        auto& obj = m_gameObjects.back();
        auto physics = m_physicsWorld->create();
        auto input = std::make_unique<InputComponent>(physics.get());

        obj.addComponent(std::move(physics));
        obj.addComponent(std::move(input));
    }
    m_gameObjects.emplace_back();
    m_gameObjects.back().addComponent(m_physicsWorld->create());
}

void GameWorld::resize(Size size) {
    Size targetSize{16, 16};
    auto sizeDiff = size / targetSize;
    auto scale = std::min(sizeDiff.x, sizeDiff.y);
    auto offset = (size - targetSize * scale) / 2.0f;

    //    Node* ptr = (Node*)b2Body_GetUserData(m_player.m_bodyId);

    m_cameraTransform.setScale(scale);
    m_cameraTransform.setPosition(offset);
}

void GameWorld::update(UpdateContext& context) {
    m_physicsWorld->update(context);

    for (auto& obj : m_gameObjects) {
        obj.update(context);
    }
};

void GameWorld::render(RenderContext& context) {
    context.clear(Colors::BLACK);
    context.setTransform(m_cameraTransform);

    for (auto& obj : m_gameObjects) {
        obj.render(context);
    }

    m_physicsWorld->render(context);
};
