#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <box2d/box2d.h>
#include <cmath>
#include <glm/common.hpp>

#include "lib.hpp"

#include "game.hpp"
#include "world.hpp"

struct AppLogUserData {
    SDL_LogOutputFunction original_output_function;
    void* original_userdata;
    void* userdata;
};

void SDL_AppLog(void* userdata, int category, SDL_LogPriority priority, const char* message) {
    auto appLogUserData = reinterpret_cast<AppLogUserData*>(userdata);
    auto app = reinterpret_cast<App*>(appLogUserData->userdata);
    if (appLogUserData->original_output_function) {
        appLogUserData->original_output_function(appLogUserData->original_userdata, category,
                                                 priority, message);
    }

    app->debugger()->log(message);
};

int SDL_Fail() {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return -1;
}

int SDL_AppInit(void** appstate, int argc, char* argv[]) {
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        return SDL_Fail();
    }

    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // set up the application data
    AppConfig config;
    config.name = version();
    config.width = 1280;
    config.height = 1024;
    config.clearColor = {194, 227, 232, 255};
    auto app = new App();
    *appstate = app;
    app->init(config);

    app->setWorld(std::make_unique<GameWorld>());

    auto log = new AppLogUserData;
    log->userdata = app;
    SDL_GetLogOutputFunction(&log->original_output_function, &log->original_userdata);
    SDL_SetLogOutputFunction(SDL_AppLog, log);

    SDL_Log("SDL: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    SDL_Log("GLM: %d.%d.%d", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_PATCH);
    {
        auto version = b2GetVersion();
        SDL_Log("Box2c: %d.%d.%d", version.major, version.minor, version.revision);
    }
    SDL_Log("Application started successfully!");
    return app->status();
}

int SDL_AppEvent(void* appstate, const SDL_Event* event) {
    auto* app = reinterpret_cast<App*>(appstate);
    app->event(event);
    return SDL_APP_CONTINUE;
}

int SDL_AppIterate(void* appstate) {
    auto* app = reinterpret_cast<App*>(appstate);
    app->iterate();
    return app->status();
}

void SDL_AppQuit(void* appstate) {
    auto* app = reinterpret_cast<App*>(appstate);
    if (app) {
        SDL_DestroyRenderer(app->renderer());
        SDL_DestroyWindow(app->window());
        delete app;
    }

    void* userdata = nullptr;
    SDL_GetLogOutputFunction(nullptr, &userdata);
    if (userdata) {
        auto log = reinterpret_cast<AppLogUserData*>(userdata);
        SDL_SetLogOutputFunction(log->original_output_function, log->original_userdata);
        delete log;
    }

    SDL_Quit();
    SDL_Log("Application quit successfully!");
}
