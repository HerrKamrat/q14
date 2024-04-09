#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cmath>

#include "game.hpp"
#include "lib.hpp"

#include "tmp.hpp"

void addNode(World& world, TextureRect textureRect) {
    auto node = std::make_unique<Node>();
    node->setColor(Colors::WHITE);
    node->setPosition({math::random(0, 300), math::random(0, 300)});
    node->setSize({math::random(10, 300), math::random(10, 300)});
    node->setAngle(math::random(0, 360));
    int i = math::random(0.0f, 10.0f);
    node->setTextureRect(textureRect);
    world.addNode(std::move(node));
}

struct AppContext {
    SDL_Window* window;
    SDL_Renderer* renderer;

    UpdateContext updateContext;
    Context context;

    SDL_bool app_quit = SDL_FALSE;
    Texture texture;
    Texture texture2;
    Texture texture3;
    std::vector<TextureRect> sprites;
    // std::vector<Node> nodes;
    World world;
};

int SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return -1;
}

int SDL_AppInit(void** appstate, int argc, char* argv[]) {
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO)) {
        return SDL_Fail();
    }

    // create a window
    SDL_Window* window = SDL_CreateWindow(version(), 640, 480, SDL_WINDOW_RESIZABLE);
    if (!window) {
        return SDL_Fail();
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL, 0);
    if (!renderer) {
        return SDL_Fail();
    }

    // print some information about the window
    SDL_ShowWindow(window);
    {
        int width, height, bbwidth, bbheight;
        SDL_GetWindowSize(window, &width, &height);
        SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
        SDL_Log("Window size: %ix%i", width, height);
        SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
        if (width != bbwidth) {
            SDL_Log("This is a highdpi environment.");
        }
    }

    std::srand(std::time(NULL));
    // set up the application data
    auto app = new AppContext{window, renderer, {}, {renderer}, false};
    *appstate = app;
    app->world = createWorld(app->updateContext, app->renderer);

    SDL_Log("Application started successfully!");

    {
        ImageInfo info{(int)gimp_image.width, (int)gimp_image.height, PixelFormat::RGBA};
        std::span<const uint8_t> s{gimp_image.pixel_data, gimp_image.width * gimp_image.height *
                                                              gimp_image.bytes_per_pixel};
        PixelRef ref{s, (int)(gimp_image.width * gimp_image.bytes_per_pixel)};

        app->texture = app->context.createTexture(info, ref);
        app->texture2 = app->context.createTexture(info, ref, {SDL_SCALEMODE_LINEAR});
    }
    {
        ImageInfo info{(int)tilemap.width, (int)tilemap.height, PixelFormat::RGBA};
        std::span<const uint8_t> s{tilemap.pixel_data,
                                   tilemap.width * tilemap.height * tilemap.bytes_per_pixel};
        PixelRef ref{s, (int)(tilemap.width * tilemap.bytes_per_pixel)};

        app->texture3 = app->context.createTexture(info, ref, {});
    }

    {
        TextureRect textureRect{app->texture3, {{0, 0}, {20, 29}}};
        for (int i = 0; i < 15; i++) {
            for (int j = 0; j < 4; j++) {
                textureRect.bounds.origin.x = 6 + 33 * i;
                textureRect.bounds.origin.y = 2 + 33 * j;
                app->sprites.push_back(textureRect);
            }
        }
    }

    addNode(app->world, randomElement(app->sprites));
    return 0;
}

float angle = 0;
bool flipX = false;
bool flipY = false;

bool lshift = false;
bool lctrl = false;

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
    auto* app = (AppContext*)appstate;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        auto sc = event->key.keysym.scancode;
        if (sc == SDL_SCANCODE_LSHIFT) {
            lshift = true;
        } else if (sc == SDL_SCANCODE_LGUI) {
            lctrl = true;
        }

    } else if (event->type == SDL_EVENT_KEY_UP) {
        auto sc = event->key.keysym.scancode;
        if (sc == SDL_SCANCODE_LSHIFT) {
            lshift = false;
        } else if (sc == SDL_SCANCODE_LGUI) {
            lctrl = false;
        } else if (sc == SDL_SCANCODE_ESCAPE) {
            app->app_quit = SDL_TRUE;
        } else if (sc == SDL_SCANCODE_D) {
            app->context.deleteTexture(app->texture);
        } else if (sc == SDL_SCANCODE_1) {
            flipX = !flipX;
        } else if (sc == SDL_SCANCODE_2) {
            flipY = !flipY;
        } else if (sc == SDL_SCANCODE_3) {
            angle -= 10;
        } else if (sc == SDL_SCANCODE_4) {
            angle += 10;
        } else if (sc == SDL_SCANCODE_RETURN) {
            addNode(app->world, randomElement(app->sprites));
        }
        if (app->world.size() > 0) {
            auto& node = *(app->world.nodeAt(app->world.size() - 1));
            if (lshift) {
                float s = 0.1f;
                if (sc == SDL_SCANCODE_LEFT) {
                    node.setScale(node.getScale() - Vec2{s, 0});
                } else if (sc == SDL_SCANCODE_RIGHT) {
                    node.setScale(node.getScale() + Vec2{s, 0});
                } else if (sc == SDL_SCANCODE_UP) {
                    node.setScale(node.getScale() + Vec2{0, s});
                } else if (sc == SDL_SCANCODE_DOWN) {
                    node.setScale(node.getScale() - Vec2{0, s});
                }
            } else if (lctrl) {
                float s = 0.1f;
                if (sc == SDL_SCANCODE_LEFT) {
                    node.setAngle(node.getAngle() - 10);
                } else if (sc == SDL_SCANCODE_RIGHT) {
                    node.setAngle(node.getAngle() + 10);
                } else if (sc == SDL_SCANCODE_UP) {
                } else if (sc == SDL_SCANCODE_DOWN) {
                }
            } else {
                if (sc == SDL_SCANCODE_LEFT) {
                    node.setPosition(node.getPosition() - Vec2{10, 0});
                } else if (sc == SDL_SCANCODE_RIGHT) {
                    node.setPosition(node.getPosition() + Vec2{10, 0});
                } else if (sc == SDL_SCANCODE_UP) {
                    node.setPosition(node.getPosition() - Vec2{0, 10});
                } else if (sc == SDL_SCANCODE_DOWN) {
                    node.setPosition(node.getPosition() + Vec2{0, 10});
                }
            }

            if (sc == SDL_SCANCODE_Z) {
                node.setZIndex(node.getZIndex() - 1);
            } else if (sc == SDL_SCANCODE_X) {
                node.setZIndex(node.getZIndex() + 1);
            }
        }
    }
    if (event->type == SDL_EVENT_QUIT) {
        app->app_quit = SDL_TRUE;
    }

    return 0;
}

uint64_t tick = 0;
int SDL_AppIterate(void* appstate) {
    auto* app = (AppContext*)appstate;
    auto& context = app->context;
    auto& world = app->world;

    UpdateContext updateContext;
    world.update(updateContext);
    uint64_t prevTick = tick;
    tick = SDL_GetTicks();
    uint64_t deltaTick = tick - prevTick;
    // draw a color
    auto time = tick / 1000.f;
    uint8_t red = (std::sin(time) + 1) / 2.0 * 255;
    uint8_t green = (std::sin(time / 2) + 1) / 2.0 * 255;
    uint8_t blue = (std::sin(time) * 2 + 1) / 2.0 * 255;

    angle += deltaTick / 10.f;

    context.setColor(Colors::WHITE);
    context.setTexture(nullptr);
    context.clear(Colors::BLACK);
    // context.clear({red, green, blue});
#if 0
    context.clear({red, green, blue});
    context.setColor({red, 0, 0});
    context.drawRect({{10, 10}, {10, 10}});
    context.setColor({0, green, 0});
    context.drawRect({20, 20, 10, 10});
    context.setColor({0, 0, blue});
    context.drawRect({30, 30, 10, 10});

    context.setColor({255, 0, 255});
    context.drawPoint({100, 100});
    context.drawLine({150, 150}, {200, 200});

    context.setColor({(uint8_t)(255 - red), (uint8_t)(255 - green), (uint8_t)(255 - blue)});
    context.setTexture(app->texture);
    context.drawRect({{100, 0}, {40, 80}});
    context.setTexture(app->texture2);
    context.drawRect({{200, 0}, {80, 40}});

    context.setTexture(app->texture3);
    context.drawRect({{0, 0}, {(float)app->texture3.width, (float)app->texture3.height}});

    Size size{32, 32};
    Size size2{32 * 2, 32 * 2};

    context.setColor(Colors::WHITE);
    context.drawRect({{0, 200}, size2}, {{0, 0}, size}, angle, flipX, flipY);

    context.setColor(Colors::WHITE);
    context.setTexture(nullptr);
#endif
    world.render(context);

    context.present();

    return app->app_quit;
}

void SDL_AppQuit(void* appstate) {
    auto* app = (AppContext*)appstate;
    if (app) {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
