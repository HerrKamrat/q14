#include "resource_loader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG

// #pragma warning(push, 0)
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-function"
#include "third_party/stb_image.h"
// #pragma GCC diagnostic pop
// #pragma warning(pop)

Image ResourceLoader::loadImage(std::span<const uint8_t> data) {
    Image image;  //{{}, {}, {nullptr}};

    auto buffer = static_cast<const stbi_uc*>(data.data());
    auto len = static_cast<int>(data.size());
    int channels = 4;

    uint8_t* img;
    int w, h;
    img = stbi_load_from_memory(buffer, len, &w, &h, nullptr, channels);

    if (!img) {
        return image;
    }

    image.info.width = w;
    image.info.height = h;

    image.pixels.data = {img, static_cast<size_t>(w * h * channels)};
    image.pixels.stride = w * 4;

    image.data = {img, stbi_image_free};

    // SDL_Log("data: %d x %d", data.size(), data.size_bytes());
    // SDL_Log("img: %d x %d x %d", w, h, n);
    return image;
}