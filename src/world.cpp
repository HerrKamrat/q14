#include "world.hpp"

#include <algorithm>

#include "lib/box2d_debug.hpp"
#include "lib/system.hpp"

#include "resources.hpp"
class GameObject;
class Component;

class Sensor {
  public:
    bool inCollision() const {
        return m_collisionCount > 0;
    }

    void onCollisionBegan() {
        SDL_Log("Collision began %d", m_collisionCount);
        m_collisionCount++;
    };

    void onCollisionEnded() {
        SDL_Log("Collision ended %d", m_collisionCount);
        m_collisionCount--;
    };

  private:
    int m_collisionCount = 0;
};

class Component {
  public:
    Component() = default;
    //~Component() = default;
    Component(const Component&) = delete;
    Component(Component&&) = default;
    Component& operator=(const Component& other) = delete;
    Component& operator=(Component&& other) = default;

    virtual ~Component() = default;
    void setGameObject(GameObject* gameObject) {
        m_gameObject = gameObject;
    };

    GameObject& getGameObject() {
        return *m_gameObject;
    }

    virtual void init(GameContext& context) {};
    virtual void deinit(GameContext& context) {};
    virtual void update(UpdateContext& context) {};
    virtual void render(RenderContext& context) {};

  private:
    GameObject* m_gameObject = nullptr;
};

class GameObject {
  public:
    GameObject() = default;
    ~GameObject() = default;
    GameObject(const GameObject&) = delete;
    GameObject(GameObject&&) = default;
    GameObject& operator=(const GameObject& other) = delete;
    GameObject& operator=(GameObject&& other) = default;

    void update(UpdateContext& context) {
        for (auto& component : m_components) {
            // component->setGameObject(this);
            component->update(context);
        }
    };

    void render(RenderContext& context) {
        context.pushTransform(m_transform);
        for (auto& component : m_components) {
            component->render(context);
        }
        context.popTransform();
    };

    void init(GameContext& context) {
        for (size_t i = 0; i < m_components.size(); i++) {
            auto component = m_components[i].get();
            component->setGameObject(this);
            component->init(context);
        }
    };

    void deinit(GameContext& context) {
        SDL_Log("Deinit %lu", m_components.size());
        for (auto& component : m_components) {
            component->deinit(context);
            component->setGameObject(nullptr);
        }
    }

    template <class T>
    T* addComponent(std::unique_ptr<T> component) {
        auto ptr = component.get();
        m_components.push_back(std::move(component));
        return ptr;
    };

    void remove() {
        SDL_Log("remove %lu", m_components.size());
        m_removed = true;
    };

    bool removed() const {
        return m_removed;
    }

    Transform& getTransform() {
        return m_transform;
    }

  private:
    std::vector<std::unique_ptr<Component>> m_components;
    bool m_removed = false;
    Transform m_transform;
};
class PhysicsBodyComponent : public Component {
  public:
    /*struct Shape {
        b2ShapeId id;
    };

    struct Sensor {
        b2ShapeId id;
        int contacts;
    };*/

    void deinit(GameContext& context) override {
        if (!B2_ID_EQUALS(m_id, b2_nullBodyId)) {
            b2DestroyBody(m_id);
            m_id = b2_nullBodyId;
        }
    };

    void onMove(Vec2 center, float rotation) {
        getGameObject().getTransform().setPosition(center);
        getGameObject().getTransform().setRotation(rotation);
    };

    void applyForce(Vec2 force) {
        b2Body_ApplyForceToCenter(m_id, {force.x, force.y}, true);
    }

    void applyImpulse(Vec2 force) {
        b2Body_ApplyLinearImpulseToCenter(m_id, {force.x, force.y}, true);
    }

    Vec2 getLinearVelocity() {
        b2Vec2 velocity = b2Body_GetLinearVelocity(m_id);
        return {velocity.x, velocity.y};
    }

    float getMass() {
        return b2Body_GetMass(m_id);
    }

    float getFriction() {
        return b2Shape_GetFriction(getShape());
    }

    void setFriction(float f) {
        b2Shape_SetFriction(getShape(), f);
    }

  private:
    b2ShapeId getShape() {
        b2ShapeId shapes = b2_nullShapeId;
        b2Body_GetShapes(m_id, &shapes, 1);
        return shapes;
    }

    // std::vector<Sensor> m_sensors;

  public:
    b2BodyId m_id;
};

class StaticBodyComponent : public PhysicsBodyComponent {
  public:
};

class DynamicBodyComponent : public PhysicsBodyComponent {
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

    Sensor m_leftSensor;
    Sensor m_rightSensor;
    Sensor m_topSensor;
    Sensor m_bottomSensor;
};

class Sprite : public Component {
  public:
    static std::unique_ptr<Sprite> create(Texture texture) {
        auto sprite = std::make_unique<Sprite>();
        TextureRect rect{texture, {{0, 0}, {texture.width, texture.height}}};
        sprite->setTextureRect(rect);
        return sprite;
    }

    void setTextureRect(TextureRect textureRect) {
        m_textureRect = textureRect;
    }

    void setFlipX(bool flipX) {
        m_flipX = flipX;
    }

    void render(RenderContext& context) override {
        // context.setTexture(m_textureRect.texture);
        // Rect rect{{-0.5f, -0.5f}, {1.0f, 1.0f}};
        // Mat3 mat = Mat3(1.0f);
        // mat[0][0] = m_flipX ? 1 : -1;
        // context.drawTexture(rect, m_textureRect.normalizedBounds(), mat);
    };

  private:
    TextureRect m_textureRect;
    bool m_flipX = false;
};

class PhysicsSystem {
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

        {
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

        {
            const Vec2 position = Vec2{15.0f + 0.5f, 8};
            const int size = 16;

            b2BodyDef groundBodyDef = b2DefaultBodyDef();
            groundBodyDef.position = {position.x, position.y};

            b2BodyId groundId = b2CreateBody(m_id, &groundBodyDef);

            b2Polygon groundBox = b2MakeBox(0.5f, 0.5f * size);

            b2ShapeDef groundShapeDef = b2DefaultShapeDef();
            // groundShapeDef.userData = ptr;
            groundShapeDef.friction = 1.0f;
            groundShapeDef.restitution = 0.0f;
            b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
        }
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

        for (auto& event : getSensorEndTouchEvents()) {
            auto userData = b2Shape_GetUserData(event.sensorShapeId);
            if (!userData) {
                continue;
            }

            // auto visitorUserData = b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));
            auto sensor = reinterpret_cast<Sensor*>(userData);
            // auto visitor = reinterpret_cast<GameObject*>(visitorUserData);
            sensor->onCollisionEnded();
        };

        for (auto& event : getMoveEvents()) {
            if (event.userData) {
                auto body = reinterpret_cast<DynamicBodyComponent*>(event.userData);
                Vec2 p = {event.transform.p.x, event.transform.p.y};
                float r = b2Rot_GetAngle(event.transform.q);
                // if (p.y > 16) {
                //     p.y -= 16;
                //     b2Body_SetTransform(event.bodyId, {p.x, p.y}, r);
                // }
                body->onMove(p, r);

                // ptr->setPosition(p);
                // ptr->setRotation(r);

                // static_cast<BodyMovementListener*>(event.userData)->onBodyMoved(p, r);
            }
        }
    };

    void render(RenderContext& context) {
        m_debugDraw.render(m_id, context);
    }

    std::unique_ptr<StaticBodyComponent> createStatic() {
        return nullptr;
    };

    std::unique_ptr<DynamicBodyComponent> createDynamicBody(Vec2 position = Vec2(0, 0)) {
        std::unique_ptr<DynamicBodyComponent> component = std::make_unique<DynamicBodyComponent>();

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.fixedRotation = true;
        bodyDef.position = {position.x, position.y};

        bodyDef.userData = component.get();
        b2BodyId bodyId = b2CreateBody(m_id, &bodyDef);

        component->m_id = bodyId;
        return component;
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

        bodyDef.userData = component.get();
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

class PlayerComponent : public Component {
  public:
    PlayerComponent(DynamicBodyComponent* physics, Sprite* sprite)
        : m_physics(physics),
          m_sprite(sprite){

          };

    void init(GameContext& context) override {
        auto p = context.physics->create();
        // p = create body
        // p.addShape(...)
        // p.addShape(..., sensor = true)
        m_physics = getGameObject().addComponent(std::move(p));
    }

    void update(UpdateContext& context) override {
        const float MAX_VELOCITY = 5.0f;
        const float VELOCITY_FORCE = 50.0f;

        auto input = context.getInputState();

        const auto velocity = body().getLinearVelocity();

        bool walking = input.left.active() || input.right.active();
        bool jump = input.up.active();
        bool pushingLeftWall = body().onLeftWall() && input.left.active();
        bool pushingRightWall = body().onRightWall() && input.right.active();
        bool pushingWall = pushingLeftWall || pushingRightWall;

        float friction = body().getFriction();
        if (body().onGround()) {
            friction = 1.0f;
        } else if (body().onWall()) {
            friction = 0.5f;
        }
        body().setFriction(friction);

        if (!walking) {
            const float T = 0.1f;
            float x = 0.0;
            if (velocity.x < -T) {
                x = 10;
            } else if (velocity.x > T) {
                x = -10;
            }
            body().applyForce({x, 0});
        } else {
            float force = VELOCITY_FORCE;
            if (body().onGround()) {
            } else if (body().onWall()) {
                force *= 0.1f;
            } else {
                // force *= 0.5f;
            }
            // b2Shape_SetFriction(m_shapeId, walking ? 0 : 1);
            if (input.left.active() && velocity.x > -MAX_VELOCITY) {
                // SDL_Log("left: %f", velocity.x);
                body().applyForce({-force, 0.0f});
            }
            if (input.right.active() && velocity.x < MAX_VELOCITY) {
                // SDL_Log("right: %f", velocity.x);
                body().applyForce({force, 0.0f});
                // b2Body_ApplyLinearImpulseToCenter(m_bodyId, {1, 0}, true);
            }
        }
        if (jump && !isJumping && (body().onGround() || pushingWall)) {
            isJumping = true;
            jumpDirection = 0;
            jumpTicks = 3;
            SDL_Log("Jump begin");
        }
        if (jump && jumpTicks > 0) {
            --jumpTicks;
            // SDL_Log("up");

            float force = body().getMass() * 2 / (1 / 60.0);
            float forceX = 0;
            if (pushingLeftWall || jumpDirection < 0) {
                forceX = force;
                jumpDirection = -1;
            } else if (pushingRightWall || jumpDirection > 0) {
                forceX = -force;
                jumpDirection = 1;
            }
            body().applyForce({forceX, -force});
            SDL_Log("Jump tick %d", jumpTicks);

            // b2Body_ApplyLinearImpulseToCenter(m_bodyId, {0, -4}, true);
        } else if (isJumping) {
            isJumping = false;
            jumpTicks = 0;
            jumpDirection = 0;
            SDL_Log("Jump end");
        }

#if 0
        if (input.left.active()) {
            m_physics->applyForce({-10.0f, 0.0f});
            m_sprite->setFlipX(true);
        } else if (input.right.active()) {
            m_physics->applyForce({10.0f, 0.0f});
            m_sprite->setFlipX(!true);
        }
#endif
    };

    DynamicBodyComponent& body() {
        return *m_physics;
    }

    DynamicBodyComponent* m_physics;
    Sprite* m_sprite;

    bool isJumping = false;
    int jumpTicks = 0;
    int jumpDirection = 0;
};

GameWorld::GameWorld() = default;

GameWorld::~GameWorld() = default;

void GameWorld::init(UpdateContext& updateContext, RenderContext& renderContext) {
    m_physics = std::make_unique<PhysicsSystem>();
    m_physics->init();

    auto img6 = ResourceLoader::loadImage(Resources::Images::Characters::Tile_0000);
    auto tex6 = renderContext.createTexture(img6.info, img6.pixels);

    for (int i = 0; i < 1; i++) {
        m_gameObjects.emplace_back();
        auto& obj = m_gameObjects.back();
        // auto physics = m_physics->create();
        auto sprite = Sprite::create(tex6);
        auto input = std::make_unique<PlayerComponent>(nullptr, sprite.get());

        // obj.addComponent(std::move(physics));
        obj.addComponent(std::move(input));
        obj.addComponent(std::move(sprite));
    }
    m_gameObjects.emplace_back();
    m_gameObjects.back().addComponent(m_physics->create());

    GameContext gc = getContext();
    for (auto& obj : m_gameObjects) {
        obj.init(gc);
    }

#if 0
    //add background
    auto sprite1 = std::make_unique<Sprite>(resourceLoader.get("bg1"));
    auto sprite2 = std::make_unique<Sprite>(resourceLoader.get("bg2"));
    auto sprite3 = std::make_unique<Sprite>(resourceLoader.get("bg3"));
    createObject(sprite1, sprite2, sprite3, std::make_unique<Background>(*sprite1, *sprite2, *sprite3));

    //add platforms
    createObject(Sprite::create("platform"), Platform::create({0,0}, {2,3}));
    createObject(Sprite::create("platform"), Platform::create({0,0}, {12,13}));

    //add objects
    createObject(Sprite::create("platform"))

    //add players
#endif
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
    GameContext gc = getContext();
    m_physics->update(context);

    for (auto& obj : m_gameObjects) {
        obj.update(context);
    }

    // std::reverse(m_gameObjects.begin(), m_gameObjects.end());

    {
        [[maybe_unused]] auto it =
            std::remove_if(m_gameObjects.begin(), m_gameObjects.end(), [&gc](GameObject& obj) {
                if (obj.removed()) {
                    obj.deinit(gc);
                    return true;
                }
                return false;
            });
        // std::for_each(it, m_gameObjects.end(), [&gc](GameObject& obj) { obj.deinit(gc); });
        // m_gameObjects.erase(it, m_gameObjects.end());
    }
};

void GameWorld::render(RenderContext& context) {
    context.clear(Colors::BLACK);
    context.setColor(Colors::WHITE);
    context.pushTransform(m_cameraTransform);

    for (auto& obj : m_gameObjects) {
        obj.render(context);
    }

    m_physics->render(context);
    context.popTransform();
}

GameContext GameWorld::getContext() {
    return {m_physics.get()};
};
