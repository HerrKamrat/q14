#pragma once

#include <algorithm>
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
    void setScaleX(float scale) {
        m_scale.x = scale;
    }
    void setScaleY(float scale) {
        m_scale.y = scale;
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
    // TODO: cache matrix
    // Mat3 m_matrix;

    Vec2 m_position{0, 0};
    Vec2 m_scale{1, 1};
    float m_rotation{0};
};

namespace math {

inline bool is_zero(float f, float T = 0.01f) {
    return f < T && f > -T;
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

template <class T>
typename T::value_type& randomElement(T& obj) {
    auto b = std::begin(obj);
    auto e = std::end(obj);
    auto d = std::distance(b, e);
    auto i = (int)(math::random() * d);
    return obj[i];
}