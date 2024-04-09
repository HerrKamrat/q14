#pragma once

#include <SDL.h>

#include <span>
#include <vector>

#include "color.hpp"
#include "math.hpp"

enum class PixelFormat {
    RGBA,
};

struct ImageInfo {
    int width;
    int height;
    PixelFormat format;
};

struct PixelRef {
    std::span<const uint8_t> data;
    int stride;
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
};

struct TextureOptions {
    SDL_ScaleMode scaleMode = SDL_SCALEMODE_NEAREST;
    SDL_BlendMode blendMode = SDL_BLENDMODE_BLEND;
};

class Context {
  public:
    Context(SDL_Renderer* renderer) : m_renderer(renderer){};

    void clear(Color color = Colors::WHITE);
    void present();

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

    void drawPoint(Vec2 point);
    void drawLine(Vec2 p0, Vec2 p1);

    Texture createTexture(ImageInfo info,
                          PixelRef pixels,
                          TextureOptions options = TextureOptions());
    void deleteTexture(Texture texture);

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
};
