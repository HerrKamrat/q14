#pragma once

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <numbers>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Size = glm::vec2;
using Mat3 = glm::mat3x3;

struct Rect {
    using value_type = Vec2::value_type;
    Vec2 origin;
    Size size;

    inline value_type left() const {
        return origin.x;
    };
    inline value_type right() const {
        return origin.x + size.x;
    };
    inline value_type top() const {
        return origin.y;
    };
    inline value_type bottom() const {
        return origin.y + size.y;
    };
    inline value_type width() const {
        return size.x;
    };
    inline value_type height() const {
        return size.y;
    };
    inline Vec2 center() const {
        return {left() + width() / 2, top() + height() / 2};
    }
    bool contains(const Vec2& p) const;
};

class Transform {
  public:
    Vec2 getPosition() const {
        return m_position;
    }
    void setPosition(Vec2 position) {
        m_position = position;
    }
    Vec2 getScale() const {
        return m_scale;
    }
    void setScale(Vec2 scale) {
        m_scale = scale;
    }
    void setScale(float scale) {
        m_scale.x = m_scale.y = scale;
    }

    float getRotation() const {
        return m_rotation;
    }
    void setRotation(float rotation) {
        m_rotation = rotation;
    }

    Mat3 getMatrix() const {
        auto t = glm::translate(glm::mat3(1.0), {m_position.x, m_position.y});
        auto r = glm::rotate(glm::mat3(1.0), m_rotation);
        auto s = glm::scale(glm::mat3(1.0), {m_scale.x, m_scale.y});
        return t * r * s;
    };

    Mat3 getInverseMatrix() const {
        return glm::inverse(getMatrix());
    }

    Vec2 transform(const Vec2 v) {
        return getMatrix() * Vec3(v, 1.0f);
    }
    Vec2 inverseTransform(const Vec2 v) {
        return getInverseMatrix() * Vec3(v, 1.0f);
    }

  private:
    Mat3 m_matrix;

    Vec2 m_position{0, 0};
    Vec2 m_scale{1, 1};
    float m_rotation{0};
};

#if 0
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

    Vec2& operator/=(Vec2 const& other) {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    Vec2& operator*=(Vec2 const& other) {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Vec2 operator/(const Vec2& other) const {
        Vec2 out = *this;
        out /= other;
        return out;
    };

    Vec2 operator*(const Vec2& other) const {
        Vec2 out = *this;
        out *= other;
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

    Vec2 rotate(float angleDegree) const {
        constexpr float pi = std::numbers::pi;
        float rad = -angleDegree / 180 * pi;
        float c = std::cos(rad);
        float s = std::sin(rad);
        float x0 = c * x - s * y;
        float y0 = s * x + c * y;
        return {x0, y0};
    }
};

struct Size {
    using value_type = float;

    value_type width;
    value_type height;
    Size& operator+=(Size const& other) {
        width += other.width;
        height += other.height;
        return *this;
    }

    Size& operator-=(Size const& other) {
        width -= other.width;
        height -= other.height;
        return *this;
    }

    Size operator+(const Size& other) const {
        Size out = *this;
        out += other;
        return out;
    };

    Size operator-(const Size& other) const {
        Size out = *this;
        out -= other;
        return out;
    };
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
#endif
namespace math {
inline float max(float a, float b) {
    return a > b ? a : b;
}
inline float min(float a, float b) {
    return a < b ? a : b;
}
float random();

template <typename T>
T random(T min, T max);
float random(float min, float max);

template <typename T>
T random(T min, T max) {
    auto r = random();
    return min * r + max * (1.0 - r);
}

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