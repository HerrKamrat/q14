#include "node.hpp"

#include <ranges>

void Node::initWithTextureRect(TextureRect textureRect) {
    setTextureRect(textureRect);
    setSize(textureRect.bounds.size);
};

void Node::update(UpdateContext& context) {
    std::stable_sort(std::begin(m_children), std::end(m_children),
                     [](const std::unique_ptr<Node>& a, const std::unique_ptr<Node>& b) {
                         return a->getZIndex() < b->getZIndex();
                     });
};

void Node::render(RenderContext& context) {
    if (!isVisible()) {
        return;
    }
    // context.setColor({255, 255, 0, 125});
    // context.setTexture(nullptr);
    // context.drawRect(visualRect());
    // context.drawRect(m_contentRect);
    // TODO: fix this
    if (getTexture().key.check > 0) {
        context.setColor(m_color);
        context.setTexture(getTexture());
        // context.drawTexture(visualRect(), m_textureRect.bounds, m_angle);

        // const Rect& rect,
        // const TextureRect& textureRect,
        // const glm::mat3x3& matri
        context.drawTexture(getContentRect(), getTextureRect().normalizedBounds(),
                            getGlobalTransform());
    }
    // context.setColor({125, 255, 0});
    // context.drawRect(getContentRect(), true);
    // context.setColor({0, 125, 255});
    // context.drawRect(visualRect(), true);

    for (auto& child : m_children) {
        child->render(context);
    }
};

Rect Node::visualRect() {
    // Size visualSize = m_contentRect.size * m_scale;
    // Vec2 center = m_contentRect.center();
    // Vec2 visualOrigin = center - (visualSize / 2.0).toVec2();
    // return {visualOrigin, visualSize};
    return m_contentRect;
}
