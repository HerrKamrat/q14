#include "game.hpp"

#include <box2d/box2d.h>

#include "lib/box2d_debug.hpp"
#include "resources.hpp"
namespace {
constexpr int kPlayerTag = 1;
Transform m_cameraTransform;

b2WorldId m_worldId;
bool m_isDebugDraw{false};
Box2dDebugDraw m_debugDraw;
b2BodyId m_bodyId;

};  // namespace

void PhysicsWorld::init(UpdateContext& updateContext, RenderContext& renderContext) {
    auto img = ResourceLoader::loadImage(Resources::Images::Tiles::Tile_0009);
    auto tex = renderContext.createTexture(img.info, img.pixels);

    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0.0f, 10.0f};
        b2WorldId worldId = m_worldId = b2CreateWorld(&worldDef);
        b2BodyDef groundBodyDef = b2DefaultBodyDef();
        groundBodyDef.position = {32.0f, 64.0f};

        b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

        b2Polygon groundBox = b2MakeBox(32.0f, 1.0f);

        b2ShapeDef groundShapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

        b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.friction = 0.3f;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {100.0f, 0.0f};

        int cols = 10;
        int rows = 10;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols - i; j++) {
                auto node = std::make_unique<Node>();
                node->setTexture(tex);
                node->setOrigin({-1, -1});
                node->setSize({2, 2});
                Node* ptr = node.get();
                addNode(std::move(node));

                bodyDef.userData = ptr;
                bodyDef.position.x = 32 - 5 + j * 2.25f + i * 1.125f;
                bodyDef.position.y = 10 - i * 2.25f;
                b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
                b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
                bodyDef.userData = nullptr;

                m_bodyId = bodyId;
            }
        }
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

void PhysicsWorld::update(UpdateContext& context) {
    World::update(context);

    // Update physics
    {
        float timeStep = 1.0f / 60.0f;
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
        for (auto& event : getBeginTouchEvents(m_worldId)) {
            auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
            auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

            // static_cast<ContactListener*>(userDataA)->onContactBegin(???);
            // static_cast<ContactListener*>(userDataB)->onContactBegin(???);
        }
        for (auto& event : getEndTouchEvents(m_worldId)) {
            auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
            auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

            // static_cast<ContactListener*>(userDataA)->onContactBegin(???);
            // static_cast<ContactListener*>(userDataB)->onContactBegin(???);
        }
        for (auto& event : getHitEvents(m_worldId)) {
            auto userDataA = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdA));
            auto userDataB = b2Body_GetUserData(b2Shape_GetBody(event.shapeIdB));

            // static_cast<ContactListener*>(userDataA)->onContactBegin(???);
            // static_cast<ContactListener*>(userDataB)->onContactBegin(???);
        }
    }

    // Handle input
    {
        const auto input = context.getInputState();
        if (input.left.active()) {
            SDL_Log("left");
            b2Body_ApplyLinearImpulseToCenter(m_bodyId, {-10, 0}, true);
        }
        if (input.right.active()) {
            SDL_Log("right");
            b2Body_ApplyLinearImpulseToCenter(m_bodyId, {10, 0}, true);
        }
        if (input.up.active()) {
            SDL_Log("up");
            b2Body_ApplyLinearImpulseToCenter(m_bodyId, {0, -10}, true);
        }
        if (input.down.active()) {
            SDL_Log("down");
            b2Body_ApplyLinearImpulseToCenter(m_bodyId, {0, 10}, true);
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
    Size targetSize{64, 64};
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