#pragma once

#include "gfx.hpp"
#if 1
class UpdateContext {};

class Node;

class World {
  public:
    void update(UpdateContext& context);
    void render(Context& context);
    void addNode(std::unique_ptr<Node> node);

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

class Node {
  public:
    void initWithTextureRect(TextureRect textureRect);
    void update(UpdateContext& context);
    void render(Context& context);

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

  protected:
  private:
    Rect visualRect();

    Color m_color{Colors::WHITE};
    TextureRect m_textureRect;
    Rect m_contentRect;
    float m_angle{0};
    Vec2 m_scale{1, 1};
    int m_zIndex;
};
#endif