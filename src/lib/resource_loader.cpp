#include "resource_loader.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#define STBI_ONLY_PNG
#include "third_party/stb_image.h"

Image ResourceLoader::loadImage(std::span<const uint8_t> data) {
    Image image;  //{{}, {}, {nullptr}};

    uint8_t* img;
    int w, h, n;
    img =
        (uint8_t*)stbi_load_from_memory((unsigned char*)&data[0], (int)data.size(), &w, &h, &n, 4);

    if (!img) {
        return image;
    }

    image.info.width = w;
    image.info.height = h;

    image.pixels.data = {img, (size_t)w * h * n};
    image.pixels.stride = w * n;

    image.data = {img, stbi_image_free};

    // SDL_Log("tiles: %d x %d", data.size(), data.size_bytes());
    // SDL_Log("img: %d x %d x %d", w, h, n);
    return image;
}