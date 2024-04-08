#pragma once

#include <cinttypes>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a = 255;
};

namespace Colors {
static Color WHITE{255, 255, 255, 255};
static Color BLACK{0, 0, 0, 255};
};  // namespace Colors
