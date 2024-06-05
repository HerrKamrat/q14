#include "gfx.hpp"

namespace {
void setDrawColor(SDL_Renderer* renderer, const Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void setTextureDrawColor(SDL_Texture* texture, const Color& color) {
    SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture, color.a);
}

SDL_FColor toFColor(const Color& color) {
    return {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
};

}  // namespace

void RenderContext::clear(Color color) {
    setDrawColor(m_renderer, color);
    SDL_RenderClear(m_renderer);
    setDrawColor(m_renderer, m_currentColor);
}

void RenderContext::present() {
    m_frameCount++;
    SDL_RenderPresent(m_renderer);
}

void RenderContext::setTransform(const Transform& transform) {
    m_transform = transform;
}

void RenderContext::setColor(Color color) {
    m_currentColor = color;
    setDrawColor(m_renderer, m_currentColor);
    if (m_currentTexture) {
        setTextureDrawColor(m_currentTexture.ptr, m_currentColor);
    }
}

void RenderContext::setTexture(Texture texture) {
    if (texture.key.check % 2 == 0) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "RenderContext::setTexture, invalid check: %d",
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

void RenderContext::drawRect(Rect rect, bool outline) {
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    auto r = reinterpret_cast<SDL_FRect*>(&rect);
    if (outline) {
        SDL_RenderRects(m_renderer, r, 1);
    } else {
        SDL_RenderFillRects(m_renderer, r, 1);
    }
}

void RenderContext::drawTexture(Rect rect, float angleDegree, bool flipX, bool flipY) {
    auto dst = reinterpret_cast<SDL_FRect*>(&rect);
    drawTexture(nullptr, dst, angleDegree, flipX, flipY);
}

void RenderContext::drawTexture(Rect rect,
                                Rect textureRect,
                                float angleDegree,
                                bool flipX,
                                bool flipY) {
    auto dst = reinterpret_cast<SDL_FRect*>(&rect);
    auto src = reinterpret_cast<SDL_FRect*>(&textureRect);
    drawTexture(src, dst, angleDegree, flipX, flipY);
}

void RenderContext::drawTexture(const Rect& rect, const Rect& uvRect, const Mat3& matrix) {
    SDL_Vertex vertices[4];
    const int indices[6] = {0, 1, 2, 2, 1, 3};

    auto m = m_transform.getMatrix() * matrix;
    auto t = uvRect;

    auto c0 = m * glm::vec3(rect.left(), rect.top(), 1);
    auto c1 = m * glm::vec3(rect.right(), rect.top(), 1);
    auto c2 = m * glm::vec3(rect.left(), rect.bottom(), 1);
    auto c3 = m * glm::vec3(rect.right(), rect.bottom(), 1);

    auto c = toFColor(m_currentColor);
    vertices[0].position = {c0.x, c0.y};
    vertices[0].tex_coord = {t.left(), t.top()};
    vertices[0].color = c;
    vertices[1].position = {c1.x, c1.y};
    vertices[1].tex_coord = {t.right(), t.top()};
    vertices[1].color = c;
    vertices[2].position = {c2.x, c2.y};
    vertices[2].tex_coord = {t.left(), t.bottom()};
    vertices[2].color = c;
    vertices[3].position = {c3.x, c3.y};
    vertices[3].tex_coord = {t.right(), t.bottom()};
    vertices[3].color = c;

    SDL_RenderGeometry(m_renderer, m_currentTexture.ptr, &vertices[0], 4, &indices[0], 6);
}

void RenderContext::drawTexture(SDL_FRect* src,
                                SDL_FRect* dst,
                                float angleDegree,
                                bool flipX,
                                bool flipY) {
    SDL_FlipMode flip = SDL_FLIP_NONE;

    if (flipX && flipY) {
        angleDegree += 180;
    } else if (flipX) {
        flip = SDL_FLIP_HORIZONTAL;
    } else if (flipY) {
        flip = SDL_FLIP_VERTICAL;
    }

    SDL_FPoint center{0, 0};
    SDL_RenderTextureRotated(m_renderer, m_currentTexture.ptr, src, dst, angleDegree, &center,
                             flip);
}

void RenderContext::drawPoint(Vec2 point) {
    auto p = reinterpret_cast<SDL_FPoint*>(&point);
    SDL_RenderPoints(m_renderer, p, 1);
}

void RenderContext::drawLine(Vec2 p0, Vec2 p1) {
    SDL_RenderLine(m_renderer, p0.x, p0.y, p1.x, p1.y);
}

void RenderContext::drawPolygon(int vertexCount,
                                bool outline,
                                std::function<void(Vertex&, int)> callback) {
    // TODO: this should work with an arbitrary number of vertices
    SDL_assert(vertexCount == 4);

    const int indices[6] = {0, 1, 2, 2, 3, 0};
    std::vector<SDL_Vertex> vertices;
    vertices.resize(vertexCount);

    Vertex vertex;
    for (int i = 0; i < vertexCount; i++) {
        vertex = {};
        callback(vertex, i);
        vertex.position = m_transform.transform(vertex.position);

        vertices[i].position.x = vertex.position.x;
        vertices[i].position.y = vertex.position.y;
        vertices[i].color = toFColor(vertex.color);
    }

    SDL_RenderGeometry(m_renderer, m_currentTexture.ptr, &vertices[0], 4, &indices[0], 6);
}

Texture RenderContext::createTexture(ImageInfo info, PixelRef pixels, TextureOptions options) {
    auto texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC,
                                     info.width, info.height);
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

void RenderContext::deleteTexture(Texture texture) {
    if (texture.key.check % 2 == 0) {
        SDL_Log("RenderContext::deleteTexture, invalid check: %d", texture.key.check);
        return;
    }

    auto& obj = m_textures.at(texture.key.index);
    if (!obj.ptr || obj.key.check != texture.key.check) {
        SDL_Log("RenderContext::deleteTexture, invalid check: %d", texture.key.check);
        return;
    }

    if (m_currentTexture.ptr == obj.ptr) {
        setTexture(nullptr);
    }

    obj.key.check += 1;
    SDL_DestroyTexture(obj.ptr);
    obj.ptr = nullptr;
}
