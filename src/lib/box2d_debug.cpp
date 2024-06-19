#include "box2d_debug.hpp"

namespace {

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
    // SDL_Log("%s, vertexCount: %d, radius: %f", __FUNCTION__, vertexCount, radius);

    Color c = Color::fromIntRGB(color);
    c.a = 125;
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
    // SDL_Log("%s", __FUNCTION__);
    auto renderer = static_cast<RenderContext*>(context);
    renderer->setColor(Color::fromIntRGB(color));
    renderer->drawPoint({p.x, p.y}, size);
}

void DrawStringFcn(b2Vec2 p, const char* s, void* context) {
    SDL_Log("%s", __FUNCTION__);
    // static_cast<RenderContext*>(context)->DrawString(p, s);
}

}  // namespace

void Box2dDebugDraw::render(b2WorldId worldId, RenderContext& context) {
    b2DebugDraw draw{DrawPolygonFcn,
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
                     true,   // contacts
                     true,   // colors
                     false,  // normals
                     false,  // impulse
                     false,  // friction
                     static_cast<void*>(&context)};

    context.setColor(Colors::WHITE);
    context.setTexture(nullptr);
    b2World_Draw(worldId, &draw);
}
