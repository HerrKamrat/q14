#pragma once

#include <memory>
#include <vector>

#include "event.hpp"
#include "gfx.hpp"
#include "input.hpp"

#include "context.hpp"

class Node;

class World {
  public:
    virtual ~World() = default;
    virtual void init(UpdateContext& updateContext, RenderContext& renderContext) = 0;
    virtual void update(UpdateContext& context) = 0;
    virtual void render(RenderContext& context) = 0;
    virtual bool isAnimating() const = 0;
    virtual void resize(Size size) = 0;
};

class Node : public EventListener {
  public:
    virtual ~Node() = default;
    void initWithTextureRect(TextureRect textureRect);
    // TEMP: made virtual for testing
    virtual void update(UpdateContext& context);
    virtual void render(RenderContext& context);

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

    void setVisible(bool visible) {
        m_visible = visible;
    }

    bool isVisible() const {
        return m_visible;
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
        setRotation(glm::radians(angleDegrees));
    }
    float getAngle() const {
        return glm::degrees(getRotation());
    }

    void setRotation(float radians) {
        m_transform.setRotation(radians);
    }
    float getRotation() const {
        return m_transform.getRotation();
    }

    Rect getContentRect() const {
        return m_contentRect;
    }

    void setPosition(Vec2 position) {
        m_transform.setPosition(position);
    }
    Vec2 getPosition() const {
        return m_transform.getPosition();
    }

    void setOrigin(Vec2 origin) {
        m_contentRect.origin = origin;
    }
    Size getOrigin() const {
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
        m_transform.setScale(scale);
    }
    Vec2 getScale() const {
        return m_transform.getScale();
    }

    void setScaleX(float scale) {
        m_transform.setScaleX(scale);
    }
    float getScaleX() {
        return m_transform.getScale().x;
    }
    void setScaleY(float scale) {
        m_transform.setScaleY(scale);
    }
    float getScaleY() {
        return getScale().y;
    }

    const Transform& getTransform() const {
        return m_transform;
    }

    Mat3 getLocalTransform() const {
        return getTransform().getMatrix();
    }

    Mat3 getGlobalTransform() const {
        if (!m_parent) {
            return getTransform().getMatrix();
        }

        return m_parent->getGlobalTransform() * getLocalTransform();
    }

    const Vec2 convertToNodeSpace(Vec2 worldPoint) const {
        return glm::inverse(getGlobalTransform()) * Vec3(worldPoint, 1.0f);
    };

    const Vec2 convertToWorldSpace(Vec2 localPoint) const {
        return getGlobalTransform() * Vec3(localPoint, 1.0f);
    };

    void addChild(std::unique_ptr<Node> child) {
        child->m_parent = this;
        m_children.push_back(std::move(child));
    }

    std::unique_ptr<Node> removeChild(Node* child) {
        if (!child || child->m_parent != this) {
            return {};
        }

        auto it = std::find_if(m_children.begin(), m_children.end(),
                               [child](std::unique_ptr<Node>& c) { return c.get() == child; });

        if (it != m_children.end()) {
            auto r = std::move(*it);
            m_children.erase(it);
            return r;
        }

        return {};
    }

    Node* getChildByTag(int tag) {
        auto it = std::find_if(m_children.begin(), m_children.end(),
                               [tag](std::unique_ptr<Node>& c) { return c->getTag() == tag; });

        if (it != m_children.end()) {
            return it->get();
        }
        return nullptr;
    }

    Node* getParent() const {
        return m_parent;
    }

    virtual Rect visualRect();

    bool contains(const Vec2& point) {
        Vec3 localPoint = m_transform.getInverseMatrix() * Vec3(point.x, point.y, 1.0f);
        return m_contentRect.contains(localPoint);
    };

    void setTag(int tag) {
        m_tag = tag;
    }

    int getTag() const {
        return m_tag;
    }

  protected:
    Node* m_parent = nullptr;
    std::vector<std::unique_ptr<Node>> m_children;

  private:
    int m_tag = 0;
    Color m_color{Colors::WHITE};
    TextureRect m_textureRect{{}, {}};
    bool m_visible{true};
    Rect m_contentRect{{0, 0}, {0, 0}};
    Transform m_transform;
    int m_zIndex{0};
};
