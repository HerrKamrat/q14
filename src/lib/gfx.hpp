#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <span>
#include <vector>

#include "color.hpp"
#include "math.hpp"

enum class PixelFormat { RGBA };

struct ImageInfo {
    int width;
    int height;
    PixelFormat format = PixelFormat::RGBA;
};

struct PixelRef {
    std::span<const uint8_t> data;
    int stride;
};

struct Image {
    ImageInfo info;
    PixelRef pixels;
    std::unique_ptr<uint8_t, void (*)(void*)> data = {nullptr, nullptr};
};

struct Texture {
    struct Id {
        uint16_t index;
        uint16_t check;
    };
    Id key;
    int width;
    int height;
};

struct TextureRect {
    Texture texture;
    Rect bounds;

    Rect normalizedBounds() const {
        int w = texture.width;
        int h = texture.height;
        return {{bounds.origin.x / w, bounds.origin.y / h}, {bounds.size.x / w, bounds.size.y / h}};
    }
};

struct TextureOptions {
    SDL_ScaleMode scaleMode = SDL_SCALEMODE_NEAREST;
    SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND;
};

struct Vertex {
    Vec2 position;
    Color color;
};

class RenderContext {
  public:
    RenderContext(SDL_Renderer* renderer) : m_renderer(renderer){};

    void clear(Color color = Colors::WHITE);
    void present();

    void setTransform(const Transform& transform);

    void setColor(Color color);
    void setTexture(Texture texture);
    void setTexture(nullptr_t) {
        m_currentTexture = {};
    };

    void drawRect(Rect rect, bool outline = false);
    void drawTexture(Rect rect, float angleDegree = 0, bool flipX = false, bool flipY = false);
    void drawTexture(Rect rect,
                     Rect textureRect,
                     float angleDegree = 0,
                     bool flipX = false,
                     bool flipY = false);

    void drawTexture(const Rect& rect, const Rect& textureRect, const Mat3& matrix);

    void drawPoint(Vec2 point);
    void drawLine(Vec2 p0, Vec2 p1);

    void drawPolygon(int vertexCount, bool outline, std::function<void(Vertex&, int)> callback);

    Texture createTexture(ImageInfo info,
                          PixelRef pixels,
                          TextureOptions options = TextureOptions());
    void deleteTexture(Texture texture);

    uint64_t frameCount() const {
        return m_frameCount;
    }

    // TODO: tmp
    SDL_Renderer* getRenderer() {
        return m_renderer;
    }

    SDL_Texture* getTexture() {
        return m_currentTexture.ptr;
    }

  private:
    void drawTexture(SDL_FRect* src, SDL_FRect* dst, float angleDegree, bool flipX, bool flipY);

    class TextureObject {
      public:
        Texture::Id key;
        SDL_Texture* ptr;
        SDL_Rect bounds;

        operator bool() const {
            return ptr != nullptr;
        }
    };

    SDL_Renderer* m_renderer;
    Color m_currentColor = Colors::WHITE;
    TextureObject m_currentTexture{};

    std::vector<TextureObject> m_textures;
    uint64_t m_frameCount;

    Transform m_transform;
};
