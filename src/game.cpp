#include "lib.hpp"
#include "tmp.hpp"

namespace {
Texture tilemapTexture;
std::vector<TextureRect> cardsTextureRects;
}  // namespace

class PlayingCard : public Node {
  public:
    virtual void onMouseButtonEvent(MouseButtonEvent& event) override {
        if (m_selected && event.released()) {
            m_selected = false;
            setColor(Colors::WHITE);
            event.stopPropagation();
            return;
        }

        if (event.pressed() && visualRect().contains(event.position())) {
            m_selected = true;
            m_dragStartPosition = getPosition();
            setColor(Colors::GREEN);
            event.stopPropagation();
            return;
        }
    };

    virtual void onMouseMotionEvent(MouseMotionEvent& event) override {
        if (m_selected) {
            setPosition(getPosition() + event.delta());
            return;
        }

        SDL_Log("onMouseMotionEvent, %fx%f", event.position().x, event.position().y);
        if (this->visualRect().contains(event.position())) {
            setColor(Colors::RED);
            event.stopPropagation();
        } else {
            setColor(Colors::WHITE);
        }
    };

  private:
    bool m_selected = false;
    Vec2 m_dragStartPosition = {0, 0};
};

World createWorld(Size size, UpdateContext& updateContext, RenderContext& renderContext) {
    {
        ImageInfo info{(int)tilemap.width, (int)tilemap.height, PixelFormat::RGBA};
        std::span<const uint8_t> s{tilemap.pixel_data,
                                   tilemap.width * tilemap.height * tilemap.bytes_per_pixel};
        PixelRef ref{s, (int)(tilemap.width * tilemap.bytes_per_pixel)};

        tilemapTexture = renderContext.createTexture(info, ref, {});
    }

    {
        TextureRect textureRect{tilemapTexture, {{0, 0}, {20, 29}}};
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 4; j++) {
                textureRect.bounds.origin.x = 6 + 33 * i;
                textureRect.bounds.origin.y = 2 + 33 * j;
                cardsTextureRects.push_back(textureRect);
            }
        }
    }

    World world;
    Vec2 origin = {1 * size.width / 2, size.height};
    for (int i = 0; i < 5; i++) {
        auto node = std::make_unique<PlayingCard>();
        auto card = randomElement(cardsTextureRects);
        node->initWithTextureRect(card);
        node->setScale(4);
        Size size = node->getSize() * node->getScale();
        Vec2 position = origin - Vec2{size.width / 2, size.height};
        float angle = -(i - 2) * 6.5f;
        position.x -= (i - 2) * (size.width * 0.75);
        position.y -= 10 - abs(i - 2) * 10;
        node->setPosition(position);
        node->setAngle(angle);
        world.addNode(std::move(node));
    }
    return std::move(world);
};