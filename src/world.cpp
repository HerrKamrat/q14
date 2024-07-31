#include "world.hpp"

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "lib/box2d_debug.hpp"

#include "resources.hpp"
class GameObject;
class Component;

namespace Components {
namespace Tags {
constexpr int BEHAVIOUR = 10;
constexpr int PHYSICS = 20;
}  // namespace Tags
}  // namespace Components
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

    const GameObject& getGameObject() const {
        return *m_gameObject;
    }

    virtual void init(GameContext& context) {};
    virtual void deinit(GameContext& context) {};
    virtual void update(GameContext& context, UpdateContext& updateContext) {};
    virtual void render(RenderContext& context) {};

    int getTag() const {
        return m_tag;
    }
    void setTag(int tag) {
        m_tag = tag;
    }

  private:
    // TODO: Replace ptr with id
    GameObject* m_gameObject = nullptr;
    int m_tag = 0;
};

class GameObject {
  public:
    GameObject() = default;
    ~GameObject() = default;
    GameObject(const GameObject&) = delete;
    GameObject(GameObject&&) = default;
    GameObject& operator=(const GameObject& other) = delete;
    GameObject& operator=(GameObject&& other) = default;

    void update(GameContext& context, UpdateContext& updateContext) {
        for (auto& component : m_components) {
            component->setGameObject(this);
            component->update(context, updateContext);
        }
    };

    void render(RenderContext& context) {
        context.pushTransform(m_transform);
        for (auto& component : m_components) {
            component->setGameObject(this);
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

    template <class T>
    T* addComponent(std::unique_ptr<T> component, int tag) {
        component->setTag(tag);
        return addComponent(std::move(component));
    };

    Component* getComponentByTag(int tag) {
        auto it = std::find_if(m_components.begin(), m_components.end(),
                               [tag](const auto& c) { return c->getTag() == tag; });
        if (it != m_components.end()) {
            return it->get();
        }
        return nullptr;
    }

    const Component* getComponentByTag(int tag) const {
        auto it = std::find_if(m_components.begin(), m_components.end(),
                               [tag](const auto& c) { return c->getTag() == tag; });
        if (it != m_components.end()) {
            return it->get();
        }
        return nullptr;
    }

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
  public:
    using Callback = std::function<void(PhysicsBodyComponent&, PhysicsBodyComponent&)>;

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

    b2ShapeId addShape(b2ShapeDef shapeDef, b2Circle circle) {
        auto shapeId = b2CreateCircleShape(m_id, &shapeDef, &circle);
        if (shapeDef.isSensor) {
            m_sensors.emplace(shapeId, Sensor{});
        }
        b2Shape_SetUserData(shapeId, this);
        return shapeId;
    }

    void onCollisionBegan(PhysicsBodyComponent& other) {
        if (m_collisionBegan) {
            m_collisionBegan(*this, other);
        }
    };

    void onCollisionEnded(PhysicsBodyComponent& other) {
        if (m_collisionEnded) {
            m_collisionEnded(*this, other);
        }
    };

    void onSensorCollisionBegan(b2ShapeId id) {
        getSensor(id).collisions++;
    };

    void onSensorCollisionEnded(b2ShapeId id) {
        getSensor(id).collisions--;
    };

    Sensor& getSensor(b2ShapeId id) {
        return m_sensors[id];
    }

    bool isSensorInCollision(b2ShapeId id) const {
        auto it = m_sensors.find(id);
        return it != m_sensors.end() && it->second.collisions > 0;
    }
    void init(GameContext& context) override {
        onMove(getPosition(), 0);
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

    Vec2 getLinearVelocity() const {
        b2Vec2 velocity = b2Body_GetLinearVelocity(m_id);
        return {velocity.x, velocity.y};
    }

    Vec2 getPosition() const {
        auto position = b2Body_GetPosition(m_id);
        return {position.x, position.y};
    }

    void setPosition(Vec2 position) {
        auto angle = b2Body_GetAngle(m_id);
        b2Body_SetTransform(m_id, {position.x, position.y}, angle);
    }

    float getMass() const {
        return b2Body_GetMass(m_id);
    }

    float getFriction() const {
        return b2Shape_GetFriction(getShape());
    }

    void setFriction(float f) {
        b2Shape_SetFriction(getShape(), f);
    }

    void setCollisionListener(Callback&& callback) {
        m_collisionBegan = callback;
    }

  private:
    b2ShapeId getShape() const {
        b2ShapeId shapes = b2_nullShapeId;
        b2Body_GetShapes(m_id, &shapes, 1);
        return shapes;
    }

    b2BodyId m_id;
    std::unordered_map<b2ShapeId, Sensor> m_sensors;

  public:
    Callback m_collisionBegan = {};
    Callback m_collisionEnded = {};
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
        Mat3 mat = Mat3(1.0f);
        mat[0][0] = m_flipX ? 1.0f : -1.0f;
        context.drawTexture(m_contentRect, m_textureRect.normalizedBounds(), mat);
    };

    void setSize(Size size) {
        m_contentRect.size = size;
    }
    Size getSize() const {
        return m_contentRect.size;
    };

    void setOrigin(Vec2 origin) {
        m_contentRect.origin = origin;
    }

  private:
    Rect m_contentRect{{-0.5f, -0.5f}, {1.0f, 1.0f}};
    TextureRect m_textureRect;
    bool m_flipX = false;
};

class TrailRendererComponent : public Component {
  public:
    // static std::unique_ptr<Sprite> create(Texture texture) {
    //     auto sprite = std::make_unique<Sprite>();
    //     TextureRect rect{texture, {{0, 0}, {texture.width, texture.height}}};
    //     sprite->setTextureRect(rect);
    //     return sprite;
    // }

    void init(GameContext& context) override {
        m_previousPosition = getGameObject().getTransform().getPosition();
    }

    void update(GameContext& context, UpdateContext& updateContext) override {
        auto p = getGameObject().getTransform().getPosition();
        m_delta = m_previousPosition - p;
        m_previousPosition = p;
    };

    void render(RenderContext& context) override {
        context.drawLine(m_delta, {0.0f, 0.0f});
        context.drawPoint({0.0f, 0.0f}, 10.0f);
        // context.setTexture(m_textureRect.texture);
        // Mat3 mat = Mat3(1.0f);
        // mat[0][0] = m_flipX ? 1.0f : -1.0f;
        // context.drawTexture(m_contentRect, m_textureRect.normalizedBounds(), mat);
    };

    void setSize(Size size) {
        m_contentRect.size = size;
    }
    Size getSize() const {
        return m_contentRect.size;
    };

    void setOrigin(Vec2 origin) {
        m_contentRect.origin = origin;
    }

  private:
    Rect m_contentRect{{-0.5f, -0.5f}, {1.0f, 1.0f}};
    Vec2 m_delta;
    Vec2 m_previousPosition;
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
            sensor->onSensorCollisionBegan(event.sensorShapeId);
        };

        for (auto& event : getSensorEndTouchEvents()) {
            auto userData = b2Shape_GetUserData(event.sensorShapeId);
            if (!userData) {
                continue;
            }
            // auto visitorUserData = b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));
            auto* sensor = reinterpret_cast<PhysicsBodyComponent*>(userData);
            // auto visitor = reinterpret_cast<GameObject*>(visitorUserData);
            sensor->onSensorCollisionEnded(event.sensorShapeId);
        };

        for (auto& event : getMoveEvents()) {
            if (event.userData) {
                auto body = reinterpret_cast<PhysicsBodyComponent*>(event.userData);
                Vec2 p = {event.transform.p.x, event.transform.p.y};
                float r = b2Rot_GetAngle(event.transform.q);
                body->onMove(p, r);
            }
        }

        for (auto& event : getBeginTouchEvents()) {
            auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
            auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

            auto bodyA = reinterpret_cast<PhysicsBodyComponent*>(userDataA);
            auto bodyB = reinterpret_cast<PhysicsBodyComponent*>(userDataB);

            bodyA->onCollisionBegan(*bodyB);
            bodyB->onCollisionBegan(*bodyA);
        }

        for (auto& event : getEndTouchEvents()) {
            auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
            auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

            auto bodyA = reinterpret_cast<PhysicsBodyComponent*>(userDataA);
            auto bodyB = reinterpret_cast<PhysicsBodyComponent*>(userDataB);

            bodyA->onCollisionEnded(*bodyB);
            bodyB->onCollisionEnded(*bodyA);
        }
    };

    void render(RenderContext& context) {
        m_debugDraw.render(m_id, context);
    }

    std::unique_ptr<PhysicsBodyComponent> createBody(b2BodyDef bodyDef) {
        b2BodyId bodyId = b2CreateBody(m_id, &bodyDef);

        std::unique_ptr<PhysicsBodyComponent> component =
            std::make_unique<PhysicsBodyComponent>(bodyId);
        b2Body_SetUserData(bodyId, component.get());
        return component;
    }

    std::unique_ptr<PhysicsBodyComponent> createBody(Vec2 position = Vec2(0, 0),
                                                     b2BodyType type = b2_dynamicBody) {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = type;
        bodyDef.position = {position.x, position.y};
        return createBody(bodyDef);
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

class BehaviourComponent : public Component {
  public:
    virtual bool moveLeft() const {
        return false;
    };
    virtual bool moveRight() const {
        return false;
    };
    virtual bool jump() const {
        return false;
    };
    virtual bool primaryAction() const {
        return false;
    }
};

class PlayerBehaviourComponent : public BehaviourComponent {
  public:
    void update(GameContext& context, UpdateContext& updateContext) override {
        m_state = updateContext.getInputState();
    }

    bool moveLeft() const override {
        return m_state.left.active();
    };
    bool moveRight() const override {
        return m_state.right.active();
    };
    bool jump() const override {
        return m_state.up.active() || m_state.secondaryAction.active();
    };
    bool primaryAction() const override {
        return m_state.primaryAction.active();
    };

  private:
    InputState m_state;
};

class BulletComponent : public Component {
  public:
    void init(GameContext& context) override {
        auto& obj = getGameObject();
        auto pc =
            static_cast<PhysicsBodyComponent*>(obj.getComponentByTag(Components::Tags::PHYSICS));
        pc->setCollisionListener([](PhysicsBodyComponent& self, PhysicsBodyComponent& other) {
            // TODO: TEMP
            self.getGameObject().remove();
            if (other.getGameObject().getComponentByTag(Components::Tags::BEHAVIOUR)) {
                other.getGameObject().remove();
            }
        });
    }
};

class EnemyBehaviourComponent : public BehaviourComponent {
  public:
    bool moveLeft() const override {
        return false;
    };
    bool moveRight() const override {
        const PhysicsBodyComponent* body = static_cast<const PhysicsBodyComponent*>(
            getGameObject().getComponentByTag(Components::Tags::PHYSICS));
        return math::is_zero(body->getLinearVelocity().y);
    };
    bool jump() const override {
        return false;  // math::random(0.0, 1.0) > 0.75;
    };
};

// TODO: rename
class PlayerComponent : public Component {
  public:
    void init(GameContext& context) override {
        m_behaviour = static_cast<BehaviourComponent*>(
            getGameObject().getComponentByTag(Components::Tags::BEHAVIOUR));
        if (!m_behaviour) {
            m_behaviour = getGameObject().addComponent(std::make_unique<BehaviourComponent>());
        }
        {
            const auto& p = getGameObject().getTransform().getPosition();
            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.position = {p.x, p.y};
            bodyDef.fixedRotation = true;
            bodyDef.type = b2_dynamicBody;

            m_physics = getGameObject().addComponent(context.physics->createBody(bodyDef),
                                                     Components::Tags::PHYSICS);
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

    void update(GameContext& context, UpdateContext& updateContext) override {
        const float MAX_VELOCITY = 5.0f;
        const float VELOCITY_FORCE = 50.0f;

        // auto input = context.getInputState();

        const auto velocity = body().getLinearVelocity();

        bool walking = behaviour().moveLeft() || behaviour().moveRight();
        bool jump = behaviour().jump();
        bool pushingLeftWall = onLeftWall() && behaviour().moveLeft();
        bool pushingRightWall = onRightWall() && behaviour().moveRight();

        // TODO: "delayed boolean"
        // pushingRightWall = delayed(onRightWall() && moveRight(), 0.5s);
        // if statement has been true for 0.5s or more, return true

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
            float x = 0.0f;
            if (velocity.x < -T) {
                x = 10.0f;
            } else if (velocity.x > T) {
                x = -10.0f;
            }
            body().applyForce({x, 0.0f});
        } else {
            float force = VELOCITY_FORCE;
            if (onGround()) {
            } else if (onWall()) {
                force *= 0.1f;
            } else {
                // force *= 0.5f;
            }
            // b2Shape_SetFriction(m_shapeId, walking ? 0 : 1);
            if (behaviour().moveLeft() && velocity.x > -MAX_VELOCITY) {
                // SDL_Log("left: %f", velocity.x);
                body().applyForce({-force, 0.0f});
            }
            if (behaviour().moveRight() && velocity.x < MAX_VELOCITY) {
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

            float force = body().getMass() * 2 / (1 / 60.0f);
            float forceX = 0;
            if (pushingLeftWall || jumpDirection < 0.0f) {
                forceX = force;
                jumpDirection = -1;
            } else if (pushingRightWall || jumpDirection > 0.0f) {
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

        if (behaviour().primaryAction()) {
            auto& obj = context.createObject();
            {
                const auto& p = getGameObject().getTransform().getPosition();
                b2BodyDef bodyDef = b2DefaultBodyDef();
                bodyDef.position = {p.x + 1.0f, p.y};
                bodyDef.fixedRotation = true;
                bodyDef.type = b2_dynamicBody;
                bodyDef.isBullet = true;
                bodyDef.gravityScale = 0.01f;
                b2Circle circle{{0.0f, 0.0f}, 0.01f};

                b2ShapeDef shapeDef = b2DefaultShapeDef();
                shapeDef.density = 1.0f;
                shapeDef.friction = 0.0f;
                shapeDef.restitution = 0.0f;

                auto pc = obj.addComponent(context.physics->createBody(bodyDef),
                                           Components::Tags::PHYSICS);
                pc->addShape(shapeDef, circle);
                pc->applyForce({1.0f, 0.0f});

                obj.addComponent(std::make_unique<BulletComponent>());
                obj.addComponent(std::make_unique<TrailRendererComponent>());
            }
        }

        if (m_sprite) {
            if (behaviour().moveLeft()) {
                m_sprite->setFlipX(true);
            } else if (behaviour().moveRight()) {
                m_sprite->setFlipX(!true);
            }
        }
    };

    PhysicsBodyComponent& body() {
        return *m_physics;
    }

    const PhysicsBodyComponent& body() const {
        return *m_physics;
    }

    const BehaviourComponent& behaviour() const {
        return *m_behaviour;
    }

    PhysicsBodyComponent* m_physics = nullptr;
    BehaviourComponent* m_behaviour = nullptr;
    Sprite* m_sprite = nullptr;

    bool isJumping = false;
    int jumpTicks = 0;
    int jumpDirection = 0;
};

GameWorld::GameWorld() = default;

GameWorld::~GameWorld() = default;

Texture tex9;
void GameWorld::init(UpdateContext& updateContext, RenderContext& renderContext) {
    m_physics = std::make_unique<PhysicsSystem>();
    m_physics->init();
    // TODO: temp
    m_gameObjects.reserve(126);

    auto img6 = ResourceLoader::loadImage(Resources::Images::Characters::Tile_0000);
    auto tex6 = renderContext.createTexture(img6.info, img6.pixels);

    auto img9 = ResourceLoader::loadImage(Resources::Images::Characters::Tile_0022);
    tex9 = renderContext.createTexture(img9.info, img9.pixels);

    auto img7 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0006);
    auto tex7 = renderContext.createTexture(img7.info, img7.pixels);

    auto img2 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0001);
    auto tex2 = renderContext.createTexture(img2.info, img2.pixels);

    auto img3 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0002);
    auto tex3 = renderContext.createTexture(img3.info, img3.pixels);

    auto img4 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0003);
    auto tex4 = renderContext.createTexture(img4.info, img4.pixels);

    auto iv0 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0020);
    auto tv0 = renderContext.createTexture(iv0.info, iv0.pixels);

    auto iv1 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0120);
    auto tv1 = renderContext.createTexture(iv1.info, iv1.pixels);

    auto iv2 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0140);
    auto tv2 = renderContext.createTexture(iv2.info, iv2.pixels);

    auto ic = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0010);
    auto tc = renderContext.createTexture(ic.info, ic.pixels);

    for (int i = 0; i < 1; i++) {
        auto& obj = m_gameObjects.emplace_back();
        obj.addComponent(std::make_unique<PlayerBehaviourComponent>(), Components::Tags::BEHAVIOUR);
        obj.addComponent(Sprite::create(tex6));
        obj.addComponent(std::make_unique<PlayerComponent>());

        obj.getTransform().setPosition({8, 14});
    }

    auto createHorizontalPlatform = [=, this](Rect rect) {
        auto& obj = m_gameObjects.emplace_back();

        b2Polygon polygon = b2MakeBox(rect.width() / 2, rect.height() / 2);
        b2ShapeDef shapeDef = b2DefaultShapeDef();

        obj.addComponent(m_physics->createBody(rect.center(), b2_staticBody))
            ->addShape(shapeDef, polygon);

        Size size{rect.size.y, rect.size.y};
        Vec2 origin = -rect.size / 2.0f;
        {
            // Left
            auto sprite = Sprite::create(tex2);
            sprite->setSize(size);
            sprite->setOrigin(origin);
            obj.addComponent(std::move(sprite));
        }
        for (int i = 2; i < rect.size.x; i++) {
            // Mid
            origin.x += 1;
            auto sprite = Sprite::create(tex3);
            sprite->setSize(size);
            sprite->setOrigin(origin);
            obj.addComponent(std::move(sprite));
        }
        {
            // Right
            origin.x += 1;
            auto sprite = Sprite::create(tex4);
            sprite->setSize(size);
            sprite->setOrigin(origin);
            obj.addComponent(std::move(sprite));
        }
        obj.getTransform().setPosition(rect.center());
    };

    auto createVerticalPlatform = [=, this](Rect rect) {
        auto& obj = m_gameObjects.emplace_back();

        b2Polygon polygon = b2MakeBox(rect.width() / 2, rect.height() / 2);
        b2ShapeDef shapeDef = b2DefaultShapeDef();

        obj.addComponent(m_physics->createBody(rect.center(), b2_staticBody))
            ->addShape(shapeDef, polygon);

        Size size{rect.size.x, rect.size.x};
        Vec2 origin = -rect.size / 2.0f;
        {
            // Top
            auto sprite = Sprite::create(tv0);
            sprite->setSize(size);
            sprite->setOrigin(origin);
            obj.addComponent(std::move(sprite));
        }
        for (int i = 2; i < rect.size.y; i++) {
            // Mid
            origin.y += 1;
            auto sprite = Sprite::create(tv1);
            sprite->setSize(size);
            sprite->setOrigin(origin);
            obj.addComponent(std::move(sprite));
        }
        {
            // Bottom
            origin.y += 1;
            auto sprite = Sprite::create(tv2);
            sprite->setSize(size);
            sprite->setOrigin(origin);
            obj.addComponent(std::move(sprite));
        }
        obj.getTransform().setPosition(rect.center());
    };

    auto createCrate = [=, this](Vec2 position) {
        Rect rect{position, {1, 1}};
        auto& obj = m_gameObjects.emplace_back();
        auto sprite = Sprite::create(tc);

        b2Polygon polygon = b2MakeBox(rect.width() / 2, rect.height() / 2);
        b2ShapeDef shapeDef = b2DefaultShapeDef();

        obj.addComponent(m_physics->createBody(rect.center(), b2_dynamicBody),
                         Components::Tags::PHYSICS)
            ->addShape(shapeDef, polygon);
        obj.addComponent(std::move(sprite));
    };

    createHorizontalPlatform({{0, 15}, {13, 1}});
    createHorizontalPlatform({{0, 12}, {8, 1}});
    createHorizontalPlatform({{0, 9}, {4, 1}});

    createVerticalPlatform({{0, 0}, {1, 16}});
    createVerticalPlatform({{15, 0}, {1, 16}});
    int s = 2;
    for (int i = 0; i < s; i++) {
        for (int j = 0; j < s - i; j++) {
            createCrate({8 + i * 0.5f + j, 8 - i});
        }
    }
    GameContext gc = getContext();
    for (auto& obj : m_gameObjects) {
        obj.init(gc);
    }
}

void GameWorld::resize(Size size) {
    Size targetSize{16, 16};
    auto sizeDiff = size / targetSize;
    auto scale = std::min(sizeDiff.x, sizeDiff.y);
    auto offset = (size - targetSize * scale) / 2.0f;

    m_cameraTransform.setScale(scale);
    m_cameraTransform.setPosition(offset);
}

float i = 2.0f;
void GameWorld::update(UpdateContext& context) {
    GameContext gc = getContext();
    m_physics->update(context);

    int count = m_gameObjects.size();
    for (int i = 0; i < count; i++) {
        auto& obj = m_gameObjects[i];
        obj.update(gc, context);
        // if (obj.getTransform().getPosition().y < 0) {
        auto cmp =
            static_cast<PhysicsBodyComponent*>(obj.getComponentByTag(Components::Tags::PHYSICS));
        if (cmp) {
            auto p = cmp->getPosition();
            if (p.y > 16) {
                p.y -= 16;
                p.x = 2;
                // p.x = 8;
                cmp->setPosition(p);
            }
        }
        //}
    }

    for (int i = count; i < (int)m_gameObjects.size(); i++) {
        auto& obj = m_gameObjects[i];
        obj.init(gc);
    }

    {
        if (context.getTime() > i && m_gameObjects.size() < 15) {
            SDL_Log("Tick");
            i += 2;
            // createEnemy();

            auto& obj = m_gameObjects.emplace_back();
            // auto physics = m_physics->create();
            auto input = std::make_unique<PlayerComponent>();

            // obj.addComponent(std::move(physics));
            obj.addComponent(std::make_unique<EnemyBehaviourComponent>(),
                             Components::Tags::BEHAVIOUR);
            obj.addComponent(std::move(input));
            obj.addComponent(Sprite::create(tex9));

            obj.getTransform().setPosition({2, 0});
            obj.init(gc);
        }
    }

    {
        [[maybe_unused]] auto it =
            std::remove_if(m_gameObjects.begin(), m_gameObjects.end(), [&gc](GameObject& obj) {
                if (obj.removed()) {
                    obj.deinit(gc);
                    return true;
                }
                return false;
            });
        std::for_each(it, m_gameObjects.end(), [&gc](GameObject& obj) { obj.deinit(gc); });
        m_gameObjects.erase(it, m_gameObjects.end());
    }
};

void GameWorld::render(RenderContext& context) {
    context.clear(Colors::BLACK);
    context.setColor(Colors::WHITE);
    context.pushTransform(m_cameraTransform);

    for (auto& obj : m_gameObjects) {
        obj.render(context);
    }

    context.setColor({255, 255, 255, 32});
    if (m_debugPhysics) {
        m_physics->render(context);
    }
    context.popTransform();
}

GameContext GameWorld::getContext() {
    return {&m_gameObjects, m_physics.get()};
};

void GameWorld::debug(Debugger& debug) {
    if (debug.pushSection("WORLD")) {
        debug.value("debug physics", m_debugPhysics);
        debug.popSection();
    }
};

GameObject& GameContext::createObject() {
    return gameObjects->emplace_back();
}
