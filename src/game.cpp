#include <random>

#include "lib.hpp"
#include "tmp.hpp"

namespace {
Texture tilemapTexture;
std::vector<TextureRect> cardsTextureRects;
}  // namespace

enum class Suit {
    None = 0,
    Hearts,
    Diamonds,
    Clubs,
    Spades,

};

enum class Rank {
    None = 0,
    Ace,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Joker,
};

struct Card {
    Suit suit;
    Rank rank;
};

class PlayingCard : public Node {
  public:
    bool contains(const Vec2& point) {
        Vec2 p0 = point - getPosition();
        p0 = p0.rotate(getAngle());
        p0 = p0 + getPosition();
        // p0 = p0 * getScale();
        return visualRect().contains(p0);
    };

    virtual void onMouseButtonEvent(MouseButtonEvent& event) override {
        if (m_selected && event.released()) {
            m_selected = false;
            setZIndex(0);
            setColor(Colors::WHITE);
            event.stopPropagation();
            return;
        }

        if (event.pressed() && contains(event.position())) {
            m_selected = true;
            setZIndex(1);
            setColor(Colors::GREEN);
            event.stopPropagation();
            return;
        }
    };

    virtual void onMouseMotionEvent(MouseMotionEvent& event) override {
        m_mouse = event.position();
        if (m_selected) {
            setPosition(getPosition() + event.delta());
            return;
        }

        SDL_Log("onMouseMotionEvent, %fx%f", event.position().x, event.position().y);
        if (contains(event.position())) {
            setColor({255, 255, 125});
            event.stopPropagation();
        } else {
            setColor(Colors::WHITE);
        }
    };

  private:
    bool m_selected = false;
    Vec2 m_mouse = {0, 0};
};

std::unique_ptr<World> createWorld(Size size,
                                   UpdateContext& updateContext,
                                   RenderContext& renderContext) {
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

    std::unique_ptr<World> world = std::make_unique<World>();

#if 0
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
#elif 0
    Vec2 origin = {size.width / 2, size.height / 2};
    auto node = std::make_unique<PlayingCard>();
    auto card = randomElement(cardsTextureRects);
    node->initWithTextureRect(card);
    node->setScale({4, 8});
    node->setAngle(-90);
    node->setPosition(origin);
    world.addNode(std::move(node));

#else
    std::vector<Card> deck;
    for (int i = 1; i < 4; i++) {
        for (int j = 1; j < 15; j++) {
            deck.push_back({(Suit)i, (Rank)j});
        }
    }

    auto rng = std::default_random_engine{};
    std::shuffle(std::begin(deck), std::end(deck), rng);

    for (auto card : deck) {
        Vec2 origin = {size.width / 2, size.height / 2};
        auto node = std::make_unique<PlayingCard>();
        auto cardTexture = cardsTextureRects.at(((int)card.suit - 1) * 15 + (int)(card.rank) - 1);
        node->initWithTextureRect(cardTexture);
        node->setScale(4);
        node->setAngle(math::random(-5, 5));
        node->setPosition(origin);
        world->addNode(std::move(node));
    }

#endif
    return std::move(world);
};