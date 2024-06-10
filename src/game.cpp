#include "game.hpp"

#include <box2d/box2d.h>

#include "lib/box2d_debug.hpp"
#include "resources.hpp"
namespace {
// constexpr int kPlayerTag = 1;
Transform m_cameraTransform;

b2WorldId m_worldId;
bool m_isDebugDraw{false};
Box2dDebugDraw m_debugDraw;

struct Player {
    b2BodyId m_bodyId;
    b2ShapeId m_shapeId;
    b2ShapeId m_feetShapeId;
    int m_groundCount = 0;
} m_player;

};  // namespace

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
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);
    // ptr->setPosition(position);

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
    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0.0f, 10.0f};
        m_worldId = b2CreateWorld(&worldDef);

        // addNode(createStaticObject(tex2, Vec2{0 + 0.5f, 31.0f + 0.5f}));
        // for (int i = 1; i < 31; i++) {
        //     addNode(createStaticObject(tex3, Vec2{i + 0.5f, 31.0f + 0.5f}));
        // }
        // addNode(createStaticObject(tex4, Vec2{31 + 0.5f, 31.0f + 0.5f}));

        addNode(createStaticObject(tex2, tex3, tex4, Vec2{15 + 0.5f, 31.0f + 0.5f}, 32));

        int cols = 10;
        int rows = 10;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols - i; j++) {
                addNode(createDynamicObject(tex1, Vec2{j + 0.5f + i * 0.5f, 30 - 2.0f * i + 0.5f}));
            }
        }
        addNode(createDynamicObject(tex5, Vec2{21 + 0.5f, 15 + 0.5f}));
    }

    {
        Vec2 position{15 + 0.5f, 30 + 0.5f};
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

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.fixedRotation = true;
        bodyDef.position = {position.x, position.y};

        bodyDef.userData = ptr;
        b2BodyId bodyId = m_player.m_bodyId = b2CreateBody(m_worldId, &bodyDef);
        m_player.m_shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

        b2Polygon feetDynamicBox = b2MakeOffsetBox(0.2f, 0.2f, {0, 0.5f}, 0);
        b2ShapeDef feetShapeDef = b2DefaultShapeDef();
        feetShapeDef.isSensor = true;

        m_player.m_feetShapeId = b2CreatePolygonShape(bodyId, &feetShapeDef, &feetDynamicBox);

        addNode(std::move(node));
    }
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

void PhysicsWorld::update(UpdateContext& context) {
    World::update(context);

    // Update physics
    {
        int subStepCount = 4;
        b2World_Step(m_worldId, context.getDeltaTime(), subStepCount);

        for (auto& event : getMoveEvents(m_worldId)) {
            if (event.userData) {
                Node* ptr = (Node*)event.userData;
                Vec2 p = {event.transform.p.x, event.transform.p.y};
                float r = b2Rot_GetAngle(event.transform.q);
                ptr->setPosition(p);
                ptr->setRotation(r);

                // static_cast<BodyMovementListener*>(event.userData)->onBodyMoved(p, r);
            }
        }
        for (auto& event : getSensorBeginTouchEvents(m_worldId)) {
            b2ShapeId m_shapeId = m_player.m_feetShapeId;
            if (B2_ID_EQUALS(event.sensorShapeId, m_shapeId)) {
                m_player.m_groundCount++;
                SDL_Log("touch ground? %d", m_player.m_groundCount);
            }
            //     auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
            //     auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

            //     static_cast<ContactListener*>(userDataA)->onContactBegin(???);
            //     static_cast<ContactListener*>(userDataB)->onContactBegin(???);
        }
        for (auto& event : getSensorEndTouchEvents(m_worldId)) {
            b2ShapeId m_shapeId = m_player.m_feetShapeId;
            if (B2_ID_EQUALS(event.sensorShapeId, m_shapeId)) {
                m_player.m_groundCount--;
                SDL_Log("leave ground? %d", m_player.m_groundCount);
            }

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
        auto m_bodyId = m_player.m_bodyId;
        auto m_shapeId = m_player.m_shapeId;

        const auto input = context.getInputState();
        b2Vec2 velocity = b2Body_GetLinearVelocity(m_bodyId);
        float MAX_VELOCITY = 5;
        float VELOCITY_FORCE = 50;

        bool walking = input.left.active() || input.right.active();

        b2Shape_SetFriction(m_shapeId, walking ? 0 : 1);
        if (input.left.active() && velocity.x > -MAX_VELOCITY) {
            SDL_Log("left: %f", velocity.x);
            b2Body_ApplyForceToCenter(m_bodyId, {-VELOCITY_FORCE, 0}, true);
        }
        if (input.right.active() && velocity.x < MAX_VELOCITY) {
            SDL_Log("right: %f", velocity.x);
            b2Body_ApplyForceToCenter(m_bodyId, {VELOCITY_FORCE, 0}, true);
            // b2Body_ApplyLinearImpulseToCenter(m_bodyId, {1, 0}, true);
        }
        if (input.up.active() && m_player.m_groundCount > 0) {
            SDL_Log("up");
            b2Body_ApplyLinearImpulseToCenter(m_bodyId, {0, -4}, true);
        }
        if (input.down.active()) {
            SDL_Log("down");
            // b2Body_ApplyLinearImpulseToCenter(m_bodyId, {0, 1}, true);
        }
        if (input.primaryAction.active()) {
            SDL_Log("primaryAction");
        }
        if (input.secondaryAction.active()) {
            SDL_Log("secondaryAction");
        }
    }
};

void PhysicsWorld::render(RenderContext& context) {
    context.setTransform(m_cameraTransform);
    World::render(context);

    if (m_isDebugDraw) {
        m_debugDraw.render(m_worldId, context);
    }
};

void PhysicsWorld::onResizeEvent(ResizeEvent& resize) {
    Size targetSize{32, 32};
    auto windowSize = resize.size();
    auto sizeDiff = windowSize / targetSize;
    auto scale = std::min(sizeDiff.x, sizeDiff.y);
    auto offset = (windowSize - targetSize * scale) / 2.0f;
    m_cameraTransform.setScale(scale);
    m_cameraTransform.setPosition(offset);
}

void PhysicsWorld::onKeyboardEvent(KeyboardEvent& event) {
    if (event.released() && event.keycode('q')) {
        m_isDebugDraw = !m_isDebugDraw;
    }
};