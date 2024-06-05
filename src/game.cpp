#include "game.hpp"
#include "resources.hpp"

#include <box2d/box2d.h>

void DrawPolygonFcn(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context) {
    // SDL_Log("%s, vertexCount: %d", __FUNCTION__, vertexCount);
    // static_cast<RenderContext*>(context)->DrawPolygon(vertices, vertexCount, color);
}

void DrawSolidPolygonFcn(b2Transform transform,
                         const b2Vec2* vertices,
                         int vertexCount,
                         float radius,
                         b2HexColor color,
                         void* context) {
    const Vec2* v = reinterpret_cast<const Vec2*>(vertices);
    Color c = Color::fromIntRGB(color);
    static_cast<RenderContext*>(context)->drawPolygon(4, true, [&](Vertex& vertex, int index) {
        auto p = b2TransformPoint(transform, vertices[index]);
        vertex.position.x = p.x;
        vertex.position.y = p.y;
        vertex.color = c;
    });
}

void DrawCircleFcn(b2Vec2 center, float radius, b2HexColor color, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawCircle(center, radius, color);
}

void DrawSolidCircleFcn(b2Transform transform, float radius, b2HexColor color, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawSolidCircle(transform, b2Vec2_zero, radius, color);
}

void DrawCapsuleFcn(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawCapsule(p1, p2, radius, color);
}

void DrawSolidCapsuleFcn(b2Vec2 p1, b2Vec2 p2, float radius, b2HexColor color, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawSolidCapsule(p1, p2, radius, color);
}

void DrawSegmentFcn(b2Vec2 p1, b2Vec2 p2, b2HexColor color, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawSegment(p1, p2, color);
}

void DrawTransformFcn(b2Transform transform, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawTransform(transform);
}

void DrawPointFcn(b2Vec2 p, float size, b2HexColor color, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawPoint(p, size, color);
}

void DrawStringFcn(b2Vec2 p, const char* s, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawString(p, s);
}

namespace {
constexpr int kPlayerTag = 1;
Transform m_cameraTransform;

b2WorldId m_worldId;
b2DebugDraw m_debugDraw;
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

        m_debugDraw = {DrawPolygonFcn,
                       DrawSolidPolygonFcn,
                       DrawCircleFcn,
                       DrawSolidCircleFcn,
                       DrawCapsuleFcn,
                       DrawSolidCapsuleFcn,
                       DrawSegmentFcn,
                       DrawTransformFcn,
                       DrawPointFcn,
                       DrawStringFcn,
                       {},
                       false,  // drawUsingBounds
                       true,   // shapes
                       true,   // joints
                       false,  // joint extras
                       false,  // aabbs
                       false,  // mass
                       false,  // contacts
                       true,   // colors
                       false,  // normals
                       false,  // impulse
                       false,  // friction
                       nullptr};
    }
}

std::span<b2BodyMoveEvent> getBodyEvents(b2WorldId worldId) {
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

        for (auto& event : getBodyEvents(m_worldId)) {
            if (event.userData) {
                Node* ptr = (Node*)event.userData;
                Vec2 p = {event.transform.p.x, event.transform.p.y};
                float r = b2Rot_GetAngle(event.transform.q);
                ptr->setPosition(p);
                ptr->setRotation(r);
            }
        }
        for (auto& event : getBeginTouchEvents(m_worldId)) {
        }
        for (auto& event : getEndTouchEvents(m_worldId)) {
        }
        for (auto& event : getHitEvents(m_worldId)) {
        }
    }

    // Handle input
    {
        const auto input = context.getInputState();
        if (input.left.active()) {
            SDL_Log("left");
            b2Body_ApplyForceToCenter(m_bodyId, {-100, 0}, true);
        }
        if (input.right.active()) {
            SDL_Log("right");
            b2Body_ApplyForceToCenter(m_bodyId, {100, 0}, true);
        }
        if (input.up.active()) {
            SDL_Log("up");
            b2Body_ApplyLinearImpulseToCenter(m_bodyId, {0, -10}, true);
        }
        if (input.down.active()) {
            SDL_Log("down");
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
    m_debugDraw.context = &context;
    b2World_Draw(m_worldId, &m_debugDraw);

    World::render(context);
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

void PhysicsWorld::onKeyboardEvent(KeyboardEvent& event) {};