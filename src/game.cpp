#include "game.hpp"

#include "lib/box2d_debug.hpp"
#include "resources.hpp"

namespace Deprecated {

namespace {
// constexpr int kPlayerTag = 1;
Transform m_cameraTransform;

b2WorldId m_worldId;
bool m_isDebugDraw{true};
Box2dDebugDraw m_debugDraw;

struct Sensor {
    int contacts = 0;
};

struct Player {
    b2BodyId m_bodyId;
    b2ShapeId m_shapeId;

    Sensor m_bottomSensor;
    Sensor m_leftSensor;
    Sensor m_rightSensor;
} m_player;

class PhysicMoveEventListener {
  public:
    virtual void onBodyMoved(Vec2 position, float rotation) = 0;
};

};  // namespace

class PhysicNode : public Node {
  public:
    virtual ~PhysicNode() = default;
};

class DynamicPhysicNode : public Node {
  public:
    virtual ~DynamicPhysicNode() = default;
};

class StaticPhysicNode : public Node {
  public:
    virtual ~StaticPhysicNode() = default;
};

std::unique_ptr<Node> createStaticObject(Texture tex, Vec2 position) {
    auto node = std::make_unique<Node>();
    node->setTexture(tex);
    node->setOrigin({-0.5f, -0.5});
    node->setSize({1, 1});
    Node* ptr = node.get();

    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = {position.x, position.y};

    b2BodyId groundId = b2CreateBody(m_worldId, &groundBodyDef);

    b2Polygon groundBox = b2MakeBox(0.5f, 0.5f);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.userData = ptr;
    groundShapeDef.friction = 1.0f;
    groundShapeDef.restitution = 0.0f;
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
    ptr->setPosition(position);

    return node;
}

std::unique_ptr<Node> createStaticObject(Texture texLeft,
                                         Texture texMiddle,
                                         Texture texRight,
                                         Vec2 position,
                                         int width) {
    auto root = std::make_unique<Node>();
    root->setTexture(texMiddle);
    root->setOrigin({-0.5f * width, -0.5});
    root->setSize({width, 1});
    Node* ptr = root.get();

    for (int i = 0; i < width; i++) {
        auto node = std::make_unique<Node>();
        node->setTexture(i == 0 ? texLeft : (i < width - 1 ? texMiddle : texRight));
        node->setOrigin({-0.5f, -0.5});
        node->setSize({1, 1});
        node->setPosition({-width * 0.5 + i + 0.5f, 0});
        root->addChild(std::move(node));
    }

    root->setPosition(position);

    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = {position.x, position.y};

    b2BodyId groundId = b2CreateBody(m_worldId, &groundBodyDef);

    b2Polygon groundBox = b2MakeBox(0.5f * width, 0.5f);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.userData = ptr;
    groundShapeDef.friction = 1.0f;
    groundShapeDef.restitution = 0.0f;
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    return root;
}

std::unique_ptr<Node> createVerticalStaticObject(Texture texFirst,
                                                 Texture texMiddle,
                                                 Texture texLast,
                                                 Vec2 position,
                                                 int size) {
    auto root = std::make_unique<Node>();
    // root->setTexture(texMiddle);
    root->setOrigin({-0.5, -0.5f * size});
    root->setSize({1, size});
    Node* ptr = root.get();

    for (int i = 0; i < size; i++) {
        auto node = std::make_unique<Node>();
        node->setTexture(i == 0 ? texFirst : (i < size - 1 ? texMiddle : texLast));
        node->setOrigin({-0.5f, -0.5});
        node->setSize({1, 1});
        node->setPosition({0, -size * 0.5 + i + 0.5f});
        root->addChild(std::move(node));
    }

    root->setPosition(position);

    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = {position.x, position.y};

    b2BodyId groundId = b2CreateBody(m_worldId, &groundBodyDef);

    b2Polygon groundBox = b2MakeBox(0.5f, 0.5f * size);

    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.userData = ptr;
    groundShapeDef.friction = 1.0f;
    groundShapeDef.restitution = 0.0f;
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    return root;
}

std::unique_ptr<Node> createDynamicObject(Texture tex, Vec2 position) {
    auto node = std::make_unique<Node>();
    node->setTexture(tex);
    node->setOrigin({-0.5f, -0.5});
    node->setSize({1, 1});
    Node* ptr = node.get();
    ptr->setPosition(position);

    b2Polygon dynamicBox = b2MakeBox(0.5f, 0.5f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {position.x, position.y};

    bodyDef.userData = ptr;
    b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);
    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

    return node;
}

void PhysicsWorld::init(UpdateContext& updateContext, RenderContext& renderContext) {
    auto img1 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0009);
    auto tex1 = renderContext.createTexture(img1.info, img1.pixels);

    auto img2 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0001);
    auto tex2 = renderContext.createTexture(img2.info, img2.pixels);

    auto img3 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0002);
    auto tex3 = renderContext.createTexture(img3.info, img3.pixels);

    auto img4 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0003);
    auto tex4 = renderContext.createTexture(img4.info, img4.pixels);

    auto img5 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0026);
    auto tex5 = renderContext.createTexture(img5.info, img5.pixels);

    auto img6 = ResourceLoader::loadImage(Resources::Images::Characters::Tile_0000);
    auto tex6 = renderContext.createTexture(img6.info, img6.pixels);

    auto img7 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0020);
    auto tex7 = renderContext.createTexture(img7.info, img7.pixels);

    auto img8 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0120);
    auto tex8 = renderContext.createTexture(img8.info, img8.pixels);

    auto img9 = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0140);
    auto tex9 = renderContext.createTexture(img9.info, img9.pixels);

    /*{
        auto root = std::make_unique<Node>();
        for (int i = 0; i < 32; i++) {
            auto node = std::make_unique<Node>();
            node->setTexture();
            node->addChild(std::move(node));
        }
        addNode(std::move(root));
    }*/

    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0.0f, 10.0f};
        m_worldId = b2CreateWorld(&worldDef);

        addNode(createStaticObject(tex2, tex3, tex4, Vec2{8, 15.0f + 0.5f}, 16));
        addNode(createVerticalStaticObject(tex7, tex8, tex9, Vec2{15 + 0.5f, 7.0f + 0.5f}, 15));
        addNode(createVerticalStaticObject(tex7, tex8, tex9, Vec2{12 + 0.5f, 11.5f + 0.5f}, 2));
        addNode(createVerticalStaticObject(tex7, tex8, tex9, Vec2{11 + 0.5f, 12.5f + 0.5f}, 2));

        int cols = 4;
        int rows = 4;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols - i; j++) {
                addNode(createDynamicObject(tex1, Vec2{j + 0.5f + i * 0.5f, 14 - 2.0f * i + 0.5f}));
            }
        }
        addNode(createDynamicObject(tex5, Vec2{6 + 0.5f, 14 + 0.5f}));
    }

    {
        Vec2 position{7 + 0.5f, 14 + 0.5f};
        auto node = std::make_unique<Node>();
        node->setTexture(tex6);
        node->setOrigin({-0.5f, -0.5});
        node->setSize({1, 1});
        Node* ptr = node.get();
        ptr->setPosition(position);

        b2Polygon dynamicBox = b2MakeBox(0.25f, 0.5f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.friction = 1.0f;
        shapeDef.restitution = 0.0f;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.fixedRotation = true;
        bodyDef.position = {position.x, position.y};

        bodyDef.userData = ptr;
        b2BodyId bodyId = m_player.m_bodyId = b2CreateBody(m_worldId, &bodyDef);
        m_player.m_shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

        {
            b2Polygon polygon = b2MakeOffsetBox(0.15f, 0.05f, {0, 0.5f}, 0);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.isSensor = true;

            shapeDef.userData = &m_player.m_bottomSensor;

            b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        }

        {
            b2Polygon polygon = b2MakeOffsetBox(0.05f, 0.45f, {0.25f, 0}, 0);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.isSensor = true;
            shapeDef.userData = &m_player.m_rightSensor;
            b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        }

        {
            b2Polygon polygon = b2MakeOffsetBox(0.05f, 0.45f, {-0.25f, 0}, 0);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.isSensor = true;
            shapeDef.userData = &m_player.m_leftSensor;
            b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
        }

        addNode(std::move(node));
    }

    /*{
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = {0, 0};

        b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);

        b2Polygon polygon = b2MakeBox(1.0f, 32.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.isSensor = true;
        b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    }*/
}

std::span<b2BodyMoveEvent> getMoveEvents(b2WorldId worldId) {
    b2BodyEvents events = b2World_GetBodyEvents(m_worldId);
    return {events.moveEvents, (size_t)events.moveCount};
}

std::span<b2ContactBeginTouchEvent> getBeginTouchEvents(b2WorldId worldId) {
    b2ContactEvents events = b2World_GetContactEvents(m_worldId);
    return {events.beginEvents, (size_t)events.beginCount};
}

std::span<b2ContactEndTouchEvent> getEndTouchEvents(b2WorldId worldId) {
    b2ContactEvents events = b2World_GetContactEvents(m_worldId);
    return {events.endEvents, (size_t)events.endCount};
}

std::span<b2ContactHitEvent> getHitEvents(b2WorldId worldId) {
    b2ContactEvents events = b2World_GetContactEvents(m_worldId);
    return {events.hitEvents, (size_t)events.hitCount};
}

std::span<b2SensorBeginTouchEvent> getSensorBeginTouchEvents(b2WorldId worldId) {
    b2SensorEvents events = b2World_GetSensorEvents(m_worldId);
    return {events.beginEvents, (size_t)events.beginCount};
}

std::span<b2SensorEndTouchEvent> getSensorEndTouchEvents(b2WorldId worldId) {
    b2SensorEvents events = b2World_GetSensorEvents(m_worldId);
    return {events.endEvents, (size_t)events.endCount};
}

bool onGround() {
    return m_player.m_bottomSensor.contacts > 0;
}

bool onLeftWall() {
    return m_player.m_leftSensor.contacts > 0;
}

bool onRightWall() {
    return m_player.m_rightSensor.contacts > 0;
}

bool onWall() {
    return onLeftWall() || onRightWall();
}

bool isJumping = false;
int jumpTicks = 0;
int jumpDirection = 0;

void PhysicsWorld::update(UpdateContext& context) {
    OldWorldImpl::update(context);

    // Update physics
    {
        int subStepCount = 4;
        b2World_Step(m_worldId, context.getDeltaTime(), subStepCount);

        for (auto& event : getMoveEvents(m_worldId)) {
            if (event.userData) {
                Node* ptr = (Node*)event.userData;
                Vec2 p = {event.transform.p.x, event.transform.p.y};
                float r = b2Rot_GetAngle(event.transform.q);
                if (p.y > 16) {
                    p.y -= 16;
                    b2Body_SetTransform(event.bodyId, {p.x, p.y}, r);
                }

                ptr->setPosition(p);
                ptr->setRotation(r);

                // static_cast<BodyMovementListener*>(event.userData)->onBodyMoved(p, r);
            }
        }
        for (auto& event : getSensorBeginTouchEvents(m_worldId)) {
            Sensor* sensor = reinterpret_cast<Sensor*>(b2Shape_GetUserData(event.sensorShapeId));
            if (sensor) {
                sensor->contacts += 1;
                // SDL_Log("leave ground? %d", sensor->contacts);
            }
            // b2ShapeId m_shapeId = m_player.m_feetShapeId;
            // if (B2_ID_EQUALS(event.sensorShapeId, m_shapeId)) {
            //     m_player.m_groundCount++;
            //     SDL_Log("touch ground? %d", m_player.m_groundCount);
            // }
            //     auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
            //     auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

            //     static_cast<ContactListener*>(userDataA)->onContactBegin(???);
            //     static_cast<ContactListener*>(userDataB)->onContactBegin(???);
        }
        for (auto& event : getSensorEndTouchEvents(m_worldId)) {
            Sensor* sensor = reinterpret_cast<Sensor*>(b2Shape_GetUserData(event.sensorShapeId));
            if (sensor) {
                sensor->contacts -= 1;
                // SDL_Log("leave ground? %d", sensor->contacts);
            }
            //            b2ShapeId m_shapeId = m_player.m_feetShapeId;
            // if (B2_ID_EQUALS(event.sensorShapeId, m_shapeId)) {
            //     m_player.m_groundCount--;
            //     SDL_Log("leave ground? %d", m_player.m_groundCount);
            // }

            //     auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
            //     auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

            //     static_cast<ContactListener*>(userDataA)->onContactBegin(???);
            //     static_cast<ContactListener*>(userDataB)->onContactBegin(???);
        }

        //     auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
        //     auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

        //     static_cast<ContactListener*>(userDataA)->onContactBegin(???);
        //     static_cast<ContactListener*>(userDataB)->onContactBegin(???);
    }

    // Handle input
    {
        const float MAX_VELOCITY = 5.0f;
        const float VELOCITY_FORCE = 50.0f;

        auto m_bodyId = m_player.m_bodyId;
        auto m_shapeId = m_player.m_shapeId;

        const auto input = context.getInputState();

        const b2Vec2 velocity = b2Body_GetLinearVelocity(m_bodyId);

        bool walking = input.left.active() || input.right.active();
        bool jump = input.up.active();
        bool pushingLeftWall = onLeftWall() && input.left.active();
        bool pushingRightWall = onRightWall() && input.right.active();
        bool pushingWall = pushingLeftWall || pushingRightWall;

        float friction = b2Shape_GetFriction(m_shapeId);
        if (onGround()) {
            friction = 1.0f;
        } else if (onWall()) {
            friction = 0.5f;
        }
        b2Shape_SetFriction(m_shapeId, friction);

        if (!walking) {
            const float T = 0.1f;
            float x = 0.0;
            if (velocity.x < -T) {
                x = 10;
            } else if (velocity.x > T) {
                x = -10;
            }
            b2Body_ApplyForceToCenter(m_bodyId, {x, 0}, true);
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
                b2Body_ApplyForceToCenter(m_bodyId, {-force, 0.0f}, true);
            }
            if (input.right.active() && velocity.x < MAX_VELOCITY) {
                // SDL_Log("right: %f", velocity.x);
                b2Body_ApplyForceToCenter(m_bodyId, {force, 0.0f}, true);
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

            float force = b2Body_GetMass(m_bodyId) * 2 / (1 / 60.0);
            float forceX = 0;
            if (pushingLeftWall || jumpDirection < 0) {
                forceX = force;
                jumpDirection = -1;
            } else if (pushingRightWall || jumpDirection > 0) {
                forceX = -force;
                jumpDirection = 1;
            }
            b2Body_ApplyForceToCenter(m_bodyId, {forceX, -force}, true);
            SDL_Log("Jump tick %d", jumpTicks);

            // b2Body_ApplyLinearImpulseToCenter(m_bodyId, {0, -4}, true);
        } else if (isJumping) {
            isJumping = false;
            jumpTicks = 0;
            jumpDirection = 0;
            SDL_Log("Jump end");
        }

        if (input.left.active()) {
            Node* ptr = (Node*)b2Body_GetUserData(m_player.m_bodyId);
            ptr->setScaleX(1);
        } else if (input.right.active()) {
            Node* ptr = (Node*)b2Body_GetUserData(m_player.m_bodyId);
            ptr->setScaleX(-1);
        }
        // if (input.down.active()) {
        //     SDL_Log("down");
        //     // b2Body_ApplyLinearImpulseToCenter(m_bodyId, {0, 1}, true);
        // }
        // if (input.primaryAction.active()) {
        //     SDL_Log("primaryAction");
        // }
        // if (input.secondaryAction.active()) {
        //     SDL_Log("secondaryAction");
        // }
        if (!true) {
            SDL_Log("========================");
            SDL_Log("right: %s", input.right.active() ? "DOWN" : "");
            SDL_Log("left: %s", input.left.active() ? "DOWN" : "");
            SDL_Log("up: %s", input.up.active() ? "DOWN" : "");
            SDL_Log("down: %s", input.down.active() ? "DOWN" : "");
            SDL_Log("primary: %s", input.primaryAction.active() ? "DOWN" : "");
            SDL_Log("secondary: %s", input.secondaryAction.active() ? "DOWN" : "");

            SDL_Log("right: %s", m_player.m_rightSensor.contacts > 0 ? "CONTACT" : "");
            SDL_Log("left: %s", m_player.m_leftSensor.contacts > 0 ? "CONTACT" : "");
            SDL_Log("bottom: %s", m_player.m_bottomSensor.contacts > 0 ? "CONTACT" : "");
            SDL_Log("bottom: %s", onGround() ? "CONTACT" : "");
            SDL_Log("========================");

            SDL_Log("velocity.x: %f", velocity.x);
            SDL_Log("velocity.y: %f", velocity.y);
            SDL_Log("friction: %f", friction);
        }
    }
};

void PhysicsWorld::render(RenderContext& context) {
    context.setTransform(m_cameraTransform);
    OldWorldImpl::render(context);

    if (m_isDebugDraw) {
        context.clear(Colors::BLACK);
        m_debugDraw.render(m_worldId, context);
    }
};

void PhysicsWorld::onResizeEvent(ResizeEvent& resize) {
    Size targetSize{16, 16};
    auto windowSize = resize.size();
    auto sizeDiff = windowSize / targetSize;
    auto scale = std::min(sizeDiff.x, sizeDiff.y);
    auto offset = (windowSize - targetSize * scale) / 2.0f;

    //    Node* ptr = (Node*)b2Body_GetUserData(m_player.m_bodyId);

    m_cameraTransform.setScale(scale);
    m_cameraTransform.setPosition(offset);
}

void PhysicsWorld::resize(Size size) {
    Size targetSize{16, 16};
    auto windowSize = size;
    auto sizeDiff = windowSize / targetSize;
    auto scale = std::min(sizeDiff.x, sizeDiff.y);
    auto offset = (windowSize - targetSize * scale) / 2.0f;

    //    Node* ptr = (Node*)b2Body_GetUserData(m_player.m_bodyId);

    m_cameraTransform.setScale(scale);
    m_cameraTransform.setPosition(offset);
}

void PhysicsWorld::onKeyboardEvent(KeyboardEvent& event) {
    if (event.released() && event.keycode('q')) {
        m_isDebugDraw = !m_isDebugDraw;
    }
};

}