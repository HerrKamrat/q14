#include "world.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "lib/box2d_debug.hpp"
#include "lib/system.hpp"

#include "resources.hpp"
class GameObject;
class Component;

template <>
struct std::equal_to<b2ShapeId> {
    constexpr bool operator()(const b2ShapeId& lhs, const b2ShapeId& rhs) const {
        return B2_ID_EQUALS(lhs, rhs);
    }
};

template <>
struct std::hash<b2ShapeId> {
    std::size_t operator()(const b2ShapeId& s) const noexcept {
        auto h = reinterpret_cast<const uint64_t*>(&s);
        return *h;
    }
};
class Component {
  public:
    Component() = default;
    virtual ~Component() = default;
    Component(const Component&) = delete;
    Component(Component&&) = default;
    Component& operator=(const Component& other) = delete;
    Component& operator=(Component&& other) = default;

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
  private:
    struct Sensor {
        int collisions = 0;
    };

  public:
    PhysicsBodyComponent(b2BodyId id) : m_id(id){};
    virtual ~PhysicsBodyComponent() {
    }

    b2ShapeId addShape(b2ShapeDef shapeDef, b2Polygon polygon) {
        auto shapeId = b2CreatePolygonShape(m_id, &shapeDef, &polygon);
        if (shapeDef.isSensor) {
            m_sensors.emplace(shapeId, Sensor{});
        }
        b2Shape_SetUserData(shapeId, this);
        return shapeId;
    }

    void onCollisionBegan(b2ShapeId id) {
        getSensor(id).collisions++;
    };

    void onCollisionEnded(b2ShapeId id) {
        getSensor(id).collisions--;
    };

    Sensor& getSensor(b2ShapeId id) {
        return m_sensors[id];
    }

    bool isSensorInCollision(b2ShapeId id) const {
        auto it = m_sensors.find(id);
        return it != m_sensors.end() && it->second.collisions > 0;
    }

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

    b2BodyId m_id;
    std::unordered_map<b2ShapeId, Sensor> m_sensors;
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
        context.setTexture(m_textureRect.texture);
        Rect rect{{-0.5f, -0.5f}, {1.0f, 1.0f}};
        Mat3 mat = Mat3(1.0f);
        mat[0][0] = m_flipX ? 1 : -1;
        context.drawTexture(rect, m_textureRect.normalizedBounds(), mat);
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

            auto sensor = reinterpret_cast<PhysicsBodyComponent*>(userData);
            // auto visitor = reinterpret_cast<GameObject*>(visitorUserData);
            sensor->onCollisionBegan(event.sensorShapeId);
        };

        for (auto& event : getSensorEndTouchEvents()) {
            auto userData = b2Shape_GetUserData(event.sensorShapeId);
            if (!userData) {
                continue;
            }
            // auto visitorUserData = b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));
            auto* sensor = reinterpret_cast<PhysicsBodyComponent*>(userData);
            // auto visitor = reinterpret_cast<GameObject*>(visitorUserData);
            sensor->onCollisionEnded(event.sensorShapeId);
        };

        for (auto& event : getMoveEvents()) {
            if (event.userData) {
                auto body = reinterpret_cast<PhysicsBodyComponent*>(event.userData);
                Vec2 p = {event.transform.p.x, event.transform.p.y};
                float r = b2Rot_GetAngle(event.transform.q);
                body->onMove(p, r);
            }
        }
    };

    void render(RenderContext& context) {
        m_debugDraw.render(m_id, context);
    }

    std::unique_ptr<PhysicsBodyComponent> createBody(Vec2 position = Vec2(0, 0)) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.fixedRotation = true;
        bodyDef.position = {position.x, position.y};

        b2BodyId bodyId = b2CreateBody(m_id, &bodyDef);

        std::unique_ptr<PhysicsBodyComponent> component =
            std::make_unique<PhysicsBodyComponent>(bodyId);
        b2Body_SetUserData(bodyId, component.get());
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
    PlayerComponent(PhysicsBodyComponent* physics, Sprite* sprite)
        : m_physics(physics),
          m_sprite(sprite){

          };

    void init(GameContext& context) override {
        {
            auto p = context.physics->createBody(getGameObject().getTransform().getPosition());
            m_physics = getGameObject().addComponent(std::move(p));
        }

        {
            b2Polygon polygon = b2MakeBox(0.25f, 0.5f);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.density = 1.0f;
            shapeDef.friction = 1.0f;
            shapeDef.restitution = 0.0f;

            m_physics->addShape(shapeDef, polygon);
        }

        {
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.isSensor = true;

            b2Polygon polygon = b2MakeOffsetBox(0.15f, 0.05f, {0, 0.5f}, 0);
            m_bottomSensor = m_physics->addShape(shapeDef, polygon);

            polygon = b2MakeOffsetBox(0.15f, 0.05f, {0, -0.5f}, 0);
            m_topSensor = m_physics->addShape(shapeDef, polygon);

            polygon = b2MakeOffsetBox(0.05f, 0.45f, {0.25f, 0}, 0);
            m_rightSensor = m_physics->addShape(shapeDef, polygon);

            polygon = b2MakeOffsetBox(0.05f, 0.45f, {-0.25f, 0}, 0);
            m_leftSensor = m_physics->addShape(shapeDef, polygon);
        }
    }

    b2ShapeId m_bottomSensor;
    b2ShapeId m_topSensor;
    b2ShapeId m_leftSensor;
    b2ShapeId m_rightSensor;

    bool onGround() {
        return body().isSensorInCollision(m_bottomSensor);
    }

    bool onLeftWall() const {
        return body().isSensorInCollision(m_leftSensor);
    }

    bool onRightWall() const {
        return body().isSensorInCollision(m_rightSensor);
    }

    bool onWall() const {
        return onLeftWall() || onRightWall();
    }

    // bool onLeft

    void update(UpdateContext& context) override {
        const float MAX_VELOCITY = 5.0f;
        const float VELOCITY_FORCE = 50.0f;

        auto input = context.getInputState();

        const auto velocity = body().getLinearVelocity();

        bool walking = input.left.active() || input.right.active();
        bool jump = input.up.active();
        bool pushingLeftWall = onLeftWall() && input.left.active();
        bool pushingRightWall = onRightWall() && input.right.active();
        bool pushingWall = pushingLeftWall || pushingRightWall;

        float friction = body().getFriction();
        if (onGround()) {
            friction = 1.0f;
        } else if (onWall()) {
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
            if (onGround()) {
            } else if (onWall()) {
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
        if (jump && !isJumping && (onGround() || pushingWall)) {
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

        if (input.left.active()) {
            m_sprite->setFlipX(true);
        } else if (input.right.active()) {
            m_sprite->setFlipX(!true);
        }
    };

    PhysicsBodyComponent& body() {
        return *m_physics;
    }

    const PhysicsBodyComponent& body() const {
        return *m_physics;
    }

    PhysicsBodyComponent* m_physics;
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

        obj.getTransform().setPosition({8, 14});
    }
    // m_gameObjects.emplace_back();
    // m_gameObjects.back().addComponent(m_physics->create());

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
