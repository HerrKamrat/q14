#pragma once

#include "event.hpp"
#include "gfx.hpp"

class UpdateContext {
  public:
    void setTicks(uint64_t ticks) {
        auto prev = m_ticks;
        m_ticks = ticks;
        m_ticksDelta = ticks - prev;
    };

    float getDeltaTime() {
        return m_ticksDelta / 1000.0f;
    }

  protected:
  private:
    uint64_t m_ticks;
    uint64_t m_ticksDelta;
};

class Node;

class World : public EventListener {
  public:
    void update(UpdateContext& context);
    void render(RenderContext& context);
    void addNode(std::unique_ptr<Node> node);
    void onMouseButtonEvent(MouseButtonEvent& event) override;
    void onMouseMotionEvent(MouseMotionEvent& event) override;

    // TODO: remove this...
    size_t size() {
        return m_nodes.size();
    }

    // TODO: remove this...
    Node* nodeAt(int index) {
        return m_nodes.at(index).get();
    }

  private:
    std::vector<std::unique_ptr<Node>> m_nodes;
};

class Node : public EventListener {
  public:
    void initWithTextureRect(TextureRect textureRect);
    void update(UpdateContext& context);
    void render(RenderContext& context);

    void setZIndex(int zIndex) {
        m_zIndex = zIndex;
    }
    int getZIndex() const {
        return m_zIndex;
    }

    void setColor(Color color) {
        m_color = color;
    };
    Color getColor() const {
        return m_color;
    }

    void setTexture(Texture texture) {
        setTexture(texture, Rect{{0, 0}, {(float)texture.width, (float)texture.height}});
    };
    void setTexture(Texture texture, Rect textureRect) {
        setTextureRect({texture, textureRect});
    };
    Texture getTexture() const {
        return m_textureRect.texture;
    };
    void setTextureRect(TextureRect textureRect) {
        m_textureRect = textureRect;
    };
    TextureRect getTextureRect() const {
        return m_textureRect;
    };
    void setAngle(float angleDegrees) {
        m_angle = angleDegrees;
    }
    float getAngle() const {
        return m_angle;
    }

    void setPosition(Vec2 position) {
        m_contentRect.origin = position;
    }
    Vec2 getPosition() const {
        return m_contentRect.origin;
    }

    void setSize(Size size) {
        m_contentRect.size = size;
    }
    Size getSize() const {
        return m_contentRect.size;
    }

    void setScale(float scale) {
        setScale({scale, scale});
    }
    void setScale(Vec2 scale) {
        m_scale = scale;
    }
    Vec2 getScale() const {
        return m_scale;
    }

    void setScaleX(float scale) {
        m_scale.x = scale;
    }
    float getScaleX() {
        return m_scale.x;
    }
    void setScaleY(float scale) {
        m_scale.y = scale;
    }
    float getScaleY() {
        return m_scale.y;
    }

    Rect visualRect();

  protected:
  private:
    Color m_color{Colors::WHITE};
    TextureRect m_textureRect;
    Rect m_contentRect;
    float m_angle{0};
    Vec2 m_scale{1, 1};
    int m_zIndex;
};
