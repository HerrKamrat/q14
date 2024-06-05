#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <box2d/box2d.h>
#include <cmath>
#include <glm/common.hpp>

#include "lib.hpp"

// #include "game.hpp"
#include "resources.hpp"
#include "test.hpp"

int SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return -1;
}

int SDL_AppInit(void** appstate, int argc, char* argv[]) {
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        return SDL_Fail();
    }

    SDL_Log("SDL: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    SDL_Log("GLM: %d.%d.%d", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_PATCH);

    {
        auto version = b2GetVersion();
        SDL_Log("Box2c: %d.%d.%d", version.major, version.minor, version.revision);
    }

    std::srand(std::time(NULL));

    // set up the application data
    auto app = new App();
    *appstate = app;
    app->init();

    app->setWorld(std::make_unique<Test>());

    SDL_Log("Application started successfully!");
    return app->status();
}

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
    auto* app = reinterpret_cast<App*>(appstate);
    app->event(event);
    return 0;
}

int SDL_AppIterate(void* appstate) {
    auto* app = reinterpret_cast<App*>(appstate);
    app->update();
    app->render();
    return app->status();
}

void SDL_AppQuit(void* appstate) {
    auto* app = reinterpret_cast<App*>(appstate);
    if (app) {
        SDL_DestroyRenderer(app->renderer());
        SDL_DestroyWindow(app->window());
        delete app;
    }

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
