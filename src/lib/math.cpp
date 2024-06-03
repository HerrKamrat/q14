#include "math.hpp"
bool Rect::contains(const Vec2& p) const {
    return p.x >= left() && p.x <= right() && p.y >= top() && p.y <= bottom();
}

#if 0

namespace {
using namespace math;
bool overlap(float aMin, float aMax, float bMin, float bMax) {
    bool overlap = !(aMax < bMin || aMin > bMax);
    return overlap;
}

bool overlapX(const Rect& a, const Rect& b) {
    return overlap(a.left(), a.right(), b.left(), b.right());
}

bool overlapY(const Rect& a, const Rect& b) {
    return overlap(a.top(), a.bottom(), b.top(), b.bottom());
}

bool collision(const Rect& a, const Rect& b) {
    return overlapY(a, b) && overlapX(a, b);
}

bool intersects(const Rect& a, const Rect& b, Intersection& intersection) {
    return overlapY(a, b) && overlapX(a, b);
}

}  // namespace


bool Rect::collision(const Rect& other) const {
    return ::collision(*this, other);
}

bool Rect::intersects(const Rect& other, Intersection* intersection = nullptr) const {
    if (!intersection) {
        return ::collision(*this, other);
    }
    return ::intersects(*this, other, *intersection);
}

#endif

float math::random() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
};

float math::random(float min, float max) {
    return min + (max - min) * random();
};
