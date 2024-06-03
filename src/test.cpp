#include "test.hpp"
#include "resources.hpp"

#include <box2d/box2d.h>

void DrawPolygonFcn(const b2Vec2* vertices, int vertexCount, b2HexColor color, void* context) {
    SDL_Log("%s, vertexCount: %d", __FUNCTION__, vertexCount);
    // static_cast<RenderContext*>(context)->DrawPolygon(vertices, vertexCount, color);
}

void DrawSolidPolygonFcn(b2Transform transform,
                         const b2Vec2* vertices,
                         int vertexCount,
                         float radius,
                         b2HexColor color,
                         void* context) {
    // SDL_Log("%s, vertexCount: %d, position: %f x %f", __FUNCTION__, vertexCount, vertices[0].x,
    // vertices[0].y);

    const Vec2* v = reinterpret_cast<const Vec2*>(vertices);

    // static_cast<RenderContext*>(context)->drawLine(b2TransformPoint(transform, v[0]), v[1]);
    // static_cast<RenderContext*>(context)->drawLine(v[1], v[2]);
    // static_cast<RenderContext*>(context)->drawLine(v[2], v[3]);
    // static_cast<RenderContext*>(context)->drawLine(v[3], v[0]);

    static_cast<RenderContext*>(context)->drawPolygon(4, true, [&](Vertex& vertex, int index) {
        auto p = b2TransformPoint(transform, vertices[index]);
        vertex.position.x = p.x;
        vertex.position.y = p.y;
        // vertex.color = {255, 255, 255, 255};
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

b2WorldId m_worldId;
b2DebugDraw m_debugDraw;
b2BodyId m_bodyId;

};  // namespace

class Player : public Node {
  public:
    Player(){
        // auto rect = std::make_unique<Node>();
        // rect->setSize({10, 10});
        // rect->setColor(Colors::BLUE);
        // addChild(std::move(rect));
    };

    virtual void update(UpdateContext& updateContext) {
        Node::update(updateContext);

        const auto& input = updateContext.getInputState();

        this->setRotation(this->getRotation() + 1.0 / 10.0 * updateContext.getDeltaTime());
        // updateContext.getPlayerInput(0);
        //  if()
    };
};

void Test::init(UpdateContext& updateContext, RenderContext& renderContext) {
    auto img = ResourceLoader::loadImage(Resources::Images::Tiles::Small);
    auto tex = renderContext.createTexture(img.info, img.pixels);
    auto node = std::make_unique<Node>();
    node->setTexture(tex);
    node->setOrigin({-1, -1});
    node->setSize({2, 2});
    node->setPosition({100, 100});
    // node->setScale(10);
    Node* ptr = node.get();
    addNode(std::move(node));

    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = (b2Vec2){0.0f, 10.0f};
        b2WorldId worldId = m_worldId = b2CreateWorld(&worldDef);
        b2BodyDef groundBodyDef = b2DefaultBodyDef();
        groundBodyDef.position = (b2Vec2){0.0f, 47.0f};

        b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

        b2Polygon groundBox = b2MakeBox(64.0f, 1.0f);

        b2ShapeDef groundShapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

        b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;
        shapeDef.friction = 0.3f;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = (b2Vec2){100.0f, 0.0f};

        int cols = 10;
        int rows = 10;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols - i; j++) {
                if ((i == (rows - 1)) && j == 0) {
                    bodyDef.userData = ptr;
                }
                bodyDef.position.x = 32 - 5 + j * 2.25f + i * 1.125f;
                bodyDef.position.y = 10 - i * 2.25f;
                b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);
                b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);
                bodyDef.userData = nullptr;
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
                       false,  // colors
                       false,  // normals
                       false,  // impulse
                       false,  // friction
                       nullptr};
    }

    if (false) {
    }

    if (false) {
        auto img = ResourceLoader::loadImage(Resources::Images::Player::Player);
        auto tex = renderContext.createTexture(img.info, img.pixels);
        auto node = std::make_unique<Player>();
        node->setTexture(tex);
        node->setSize({100, 100});
        node->setPosition({100, 0});
        node->setAngle(45);
        node->setTag(kPlayerTag);
        addNode(std::move(node));
    }
}

void Test::update(UpdateContext& context) {
    World::update(context);

    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;

    b2World_Step(m_worldId, timeStep, subStepCount);

    // b2Vec2 position = b2Body_GetPosition(m_bodyId);
    // float angle = b2Body_GetAngle(m_bodyId);
    // SDL_Log("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
};

void Test::render(RenderContext& context) {
    Transform transform;
    transform.setScale(10);
    context.setTransform(transform);
    m_debugDraw.context = &context;
    b2World_Draw(m_worldId, &m_debugDraw);
    b2BodyEvents events = b2World_GetBodyEvents(m_worldId);
    for (int i = 0; i < events.moveCount; i++) {
        auto event = events.moveEvents[i];
        if (event.userData) {
            Node* ptr = (Node*)event.userData;
            Vec2 p = {event.transform.p.x, event.transform.p.y};
            float r = b2Rot_GetAngle(event.transform.q);
            ptr->setPosition(p);
            ptr->setRotation(r);
        }
    }

    World::render(context);
};

void Test::onKeyboardEvent(KeyboardEvent& event) {

};