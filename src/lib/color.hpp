#pragma once

#include <cinttypes>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a = 255;

    static Color fromIntRGBA(int hexColor) {
        Color c;
        c.r = (hexColor >> 24) & 0xFF;
        c.g = (hexColor >> 16) & 0xFF;
        c.b = (hexColor >> 8) & 0xFF;
        c.a = (hexColor >> 0) & 0xFF;
        return c;
    }

    static Color fromIntRGB(int hexColor) {
        Color c;
        c.r = (hexColor >> 16) & 0xFF;
        c.g = (hexColor >> 8) & 0xFF;
        c.b = (hexColor >> 0) & 0xFF;
        c.a = 255;
        return c;
    }
};

namespace Colors {
static Color WHITE{255, 255, 255, 255};
static Color BLACK{0, 0, 0, 255};
static Color RED{255, 0, 0, 255};
static Color GREEN{0, 255, 0, 255};
static Color BLUE{0, 0, 255, 255};
};  // namespace Colors
