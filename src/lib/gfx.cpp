#include "gfx.hpp"

namespace {
void setDrawColor(SDL_Renderer* renderer, const Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void setTextureDrawColor(SDL_Texture* texture, const Color& color) {
    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture, color.a);
}
}  // namespace

void Context::clear(Color color) {
    setDrawColor(m_renderer, color);
    SDL_RenderClear(m_renderer);
    setDrawColor(m_renderer, m_currentColor);
}

void Context::present() {
    SDL_RenderPresent(m_renderer);
}

void Context::setColor(Color color) {
    m_currentColor = color;
    setDrawColor(m_renderer, m_currentColor);
    if (m_currentTexture) {
        setTextureDrawColor(m_currentTexture.ptr, m_currentColor);
    }
}

void Context::setTexture(Texture texture) {
    if (texture.key.check % 2 == 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Context::setTexture, invalid check: %d",
                    texture.key.check);
        return;
    }

    auto obj = m_textures.at(texture.key.index);
    if (!obj.ptr || obj.key.check != texture.key.check) {
        return;
    }
    m_currentTexture = obj;
    if (m_currentTexture) {
        setTextureDrawColor(m_currentTexture.ptr, m_currentColor);
    }
}

void Context::drawRect(Rect rect) {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    auto r = reinterpret_cast<SDL_FRect*>(&rect);
    if (m_currentTexture) {
        SDL_RenderTexture(m_renderer, m_currentTexture.ptr, nullptr, r);
    } else {
        SDL_RenderFillRects(m_renderer, r, 1);
    }
}

void Context::drawRect(Rect rect, Rect textureRect, float angleDegree, bool flipX, bool flipY) {
    auto dst = reinterpret_cast<SDL_FRect*>(&rect);
    auto src = reinterpret_cast<SDL_FRect*>(&textureRect);

    SDL_FlipMode flip = SDL_FLIP_NONE;

    if (flipX && flipY) {
        angleDegree += 180;
    } else if (flipX) {
        flip = SDL_FLIP_HORIZONTAL;
    } else if (flipY) {
        flip = SDL_FLIP_VERTICAL;
    }

    SDL_RenderTextureRotated(m_renderer, m_currentTexture.ptr, src, dst, angleDegree, nullptr,
                             flip);
}

void Context::drawPoint(Vec2 point) {
    auto p = reinterpret_cast<SDL_FPoint*>(&point);
    SDL_RenderPoints(m_renderer, p, 1);
}

void Context::drawLine(Vec2 p0, Vec2 p1) {
    SDL_RenderLine(m_renderer, p0.x, p0.y, p1.x, p1.y);
}

Texture Context::createTexture(ImageInfo info, PixelRef pixels, TextureOptions options) {
    auto texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
                                     info.width, info.height);
    SDL_Log("create texture %d, %d, %d", info.width, info.height, pixels.stride);
    if (texture == NULL) {
        return {{0, 0}, 0, 0};
    }

    SDL_Rect rect = {0, 0, info.width, info.height};
    SDL_UpdateTexture(texture, &rect, std::data(pixels.data), pixels.stride);

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    SDL_SetTextureScaleMode(texture, options.scaleMode);
    SDL_SetTextureBlendMode(texture, options.blendMode);

    TextureObject* obj = nullptr;
    {
        auto it = std::find_if(std::begin(m_textures), std::end(m_textures),
                               [](const TextureObject& obj) { return obj.key.check % 2 == 0; });
        if (it < std::end(m_textures)) {
            obj = &*it;
        } else {
            Texture::Id key = {0, 0};
            key.index = m_textures.size();

            m_textures.push_back({key});
            obj = &m_textures.back();
        }
    }
    obj->key.check += 1;
    obj->ptr = texture;
    obj->bounds = rect;

    Texture tex = {obj->key, rect.w, rect.h};
    return tex;
}

void Context::deleteTexture(Texture texture) {
    if (texture.key.check % 2 == 0) {
        SDL_Log("Context::deleteTexture, invalid check: %d", texture.key.check);
        return;
    }

    auto& obj = m_textures.at(texture.key.index);
    if (!obj.ptr || obj.key.check != texture.key.check) {
        SDL_Log("Context::deleteTexture, invalid check: %d", texture.key.check);
        return;
    }

    if (m_currentTexture.ptr == obj.ptr) {
        setTexture(nullptr);
    }

    obj.key.check += 1;
    SDL_DestroyTexture(obj.ptr);
    obj.ptr = nullptr;
}
