#pragma once

#include <math.h>
#include <stdint.h>
#include <cstdlib>

struct Intersection;
struct Size;
struct Vec2 {
    using value_type = float;

    value_type x;
    value_type y;

    Vec2& operator+=(Vec2 const& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(Vec2 const& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2 operator+(const Vec2& other) const {
        Vec2 out = *this;
        out += other;
        return out;
    };

    Vec2 operator-(const Vec2& other) const {
        Vec2 out = *this;
        out -= other;
        return out;
    };

    Vec2 operator*(const value_type& other) const {
        Vec2 out = *this;
        out.x *= other;
        out.y *= other;
        return out;
    };
    Vec2 operator/(const value_type& other) const {
        Vec2 out = *this;
        out.x /= other;
        out.y /= other;
        return out;
    };

    value_type length() const {
        return sqrtf(x * x + y * y);
    }

    Vec2 normalize() const {
        auto l = length();
        if (l > 0) {
            return {x / l, y / l};
        }
        return {1, 0};
    }
};

struct Size {
    using value_type = float;

    value_type width;
    value_type height;

    Size operator*(const value_type& other) const {
        Size out = *this;
        out.width *= other;
        out.height *= other;
        return out;
    };
    Size operator/(const value_type& other) const {
        Size out = *this;
        out.width /= other;
        out.height /= other;
        return out;
    };
    Size operator*(const Vec2& other) const {
        Size out = *this;
        out.width *= other.x;
        out.height *= other.y;
        return out;
    };
    Size operator/(const Vec2& other) const {
        Size out = *this;
        out.width /= other.x;
        out.height /= other.y;
        return out;
    };

    Vec2 toVec2() const {
        return {width, height};
    }
};

struct Range {
    using value_type = float;

    value_type start;
    value_type end;
};

struct Rect {
    using value_type = Vec2::value_type;

    Vec2 origin;
    Size size;

    inline value_type left() const {
        return origin.x;
    };
    inline value_type right() const {
        return origin.x + size.width;
    };
    inline value_type top() const {
        return origin.y;
    };
    inline value_type bottom() const {
        return origin.y + size.height;
    };
    inline value_type width() const {
        return size.width;
    };
    inline value_type height() const {
        return size.height;
    };
    inline Vec2 center() const {
        return {left() + width() / 2, top() + height() / 2};
    }

    bool contains(const Vec2& p) const;
    bool intersects(const Rect& other, Intersection* intersection) const;
    bool collision(const Rect& other) const;
};

struct Intersection {
    Vec2 position;
    Vec2 normal;
};

namespace math {
inline float max(float a, float b) {
    return a > b ? a : b;
}
inline float min(float a, float b) {
    return a < b ? a : b;
}
float random();

float random(float min, float max);

}  // namespace math

#include <algorithm>
template <class T>
typename T::value_type& randomElement(T& obj) {
    auto b = std::begin(obj);
    auto e = std::end(obj);
    auto d = std::distance(b, e);
    auto i = (int)(math::random() * d);
    return obj[i];
}