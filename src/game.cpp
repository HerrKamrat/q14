#include <random>

#include "config.hpp"
#include "lib.hpp"
#include "tmp.hpp"

namespace {
Texture tilemapTexture;
std::vector<TextureRect> cardsTextureRects;
TextureRect backside;
TextureRect outline;
TextureRect mask;
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

class TouchHelper {
  public:
    std::function<void(void)> onTap = {};
    std::function<void(void)> onDrag = {};
};

class PlayingCard : public Node {
  public:
    PlayingCard() {
    }

    void init(TextureRect front, TextureRect back, TextureRect outline, TextureRect mask) {
        initWithTextureRect(back);
        setOrigin({0, 0});
        m_front = front;
        m_back = back;
        m_outline = outline;
        m_mask = mask;
        auto child = std::make_unique<Node>();
        child->initWithTextureRect(m_outline);
        child->setColor(Colors::RED);
        child->setPosition({-1, -1});
        child->setVisible(false);

        m_outlineNode = child.get();

        addChild(std::move(child));
    }

    virtual void update(UpdateContext& context) override {
        // setAngle(getAngle() + 1);
        Node::update(context);
    };

    void flip() {
        m_revealed = !m_revealed;
        setTextureRect(m_revealed ? m_front : m_back);
    }

    virtual void onMouseButtonEvent(MouseButtonEvent& event) override {
        if (m_selected && event.released()) {
            if (!m_dragged) {
                flip();
            }
            m_dragged = false;
            m_selected = false;
            setZIndex(0);
            m_outlineNode->setColor(Colors::BLUE);
            event.stopPropagation();
            return;
        }

        if (event.pressed() && contains(event.position())) {
            m_selected = true;
            setZIndex(1);
            m_outlineNode->setColor(Colors::GREEN);
            event.stopPropagation();
            return;
        }
    };

    virtual void onMouseMotionEvent(MouseMotionEvent& event) override {
        if (m_selected) {
            setPosition(getPosition() + event.delta());
            m_dragged = true;
            return;
        }

        if (contains(event.position())) {
            m_hovering = true;
            m_outlineNode->setVisible(true);
            m_outlineNode->setColor(Colors::BLUE);
            event.stopPropagation();
        } else if (m_hovering) {
            m_hovering = false;
            m_outlineNode->setVisible(false);
        }
    };

  private:
    bool m_selected = false;
    bool m_revealed = false;
    bool m_dragged = true;
    bool m_hovering = false;

    TextureRect m_front;
    TextureRect m_back;
    TextureRect m_outline;
    TextureRect m_mask;

    Node* m_outlineNode;
};

#ifndef RUN_TEST
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
        backside = {tilemapTexture, {{6 + 33 * 14, 2 + 33 * 4}, {20, 29}}};
        mask = {tilemapTexture, {{6 + 33 * 14, 2 + 33 * 9}, {20, 29}}};
        outline = {tilemapTexture, {{6 + 33 * 13 - 1, 2 + 33 * 9 - 1}, {20 + 2, 29 + 2}}};
    }

    std::unique_ptr<World> world = std::make_unique<World>();

    std::vector<Card> deck;
    for (int i = 1; i < 4; i++) {
        for (int j = 1; j < 15; j++) {
            deck.push_back({(Suit)i, (Rank)j});
        }
    }

    auto rng = std::default_random_engine{};
    std::shuffle(std::begin(deck), std::end(deck), rng);

    for (auto card : deck) {
        Vec2 origin = {size.x / 2, size.y / 2};
        auto node = std::make_unique<PlayingCard>();
        auto cardTexture = cardsTextureRects.at(((int)card.suit - 1) * 15 + (int)(card.rank) - 1);
        node->init(cardTexture, backside, outline, mask);
        node->setScale(4);
        node->setAngle(math::random(-5, 5));
        node->setPosition(origin);
        world->addNode(std::move(node));
    }
    return std::move(world);
};
#endif