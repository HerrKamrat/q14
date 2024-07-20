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
void test();
int SDL_AppInit(void** appstate, int argc, char* argv[]) {
    // init the library, here we make a window so we only need the Video capabilities.
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD)) {
        return SDL_Fail();
    }
    // test();
    // return -1;

    SDL_Log("SDL: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    SDL_Log("GLM: %d.%d.%d", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_PATCH);
    {
        auto version = b2GetVersion();
        SDL_Log("Box2c: %d.%d.%d", version.major, version.minor, version.revision);
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

struct Id {
    uint16_t index;
    uint16_t check;

    bool valid() const {
        return check % 2 != 0;
    }

    bool operator==(const Id& other) const {
        return index == other.index && check == other.check;
    }

    // bool operator!=(const Id& other) {
    //     return !(*this == other);
    // }
};

static Id null_id = {0, 0};

template <class T>
class System {
  public:
    struct Entry {
        Id key;
        T object;
        bool valid() const {
            return key.valid();
        }
    };

    std::vector<Entry> m_entries;

    bool valid(Id id) const {
        return id.valid() && id.index >= 0 && id.index < m_entries.size();
    }

    Id create() {
        Id id = null_id;
        {
            Entry* entry = nullptr;

            auto it = std::find_if(std::begin(m_entries), std::end(m_entries),
                                   [](const auto& entry) { return !entry.key.valid(); });
            if (it < std::end(m_entries)) {
                SDL_Log("Reuse obj");
                entry = &*it;
            } else {
                SDL_Log("Create obj");

                Id key = {0, 0};
                T obj;
                key.index = static_cast<uint16_t>(m_entries.size());

                m_entries.push_back({key, obj});
                entry = &m_entries.back();
            }
            entry->key.check += 1;
            id = entry->key;
        }
        return id;
    }

    void free(Id id) {
        if (!valid(id)) {
            return;
        }

        auto& entry = m_entries.at(id.index);
        if (entry.key != id) {
            return;
        }

        // TODO: delete entry.obj
        entry.key.check += 1;
    }

    T* get(Id id) {
        if (!valid(id)) {
            return nullptr;
        }

        auto& entry = m_entries.at(id.index);
        if (entry.key != id) {
            return nullptr;
        }

        return &entry.object;
    }

    void update() {
        for (auto& entry : m_entries) {
            if (!entry.valid()) {
                continue;
            }
            SDL_Log("Update #%d", entry.key.index);
        }
    }
};

struct Velocity {
    float x = 0;
    float y = 0;
};

void test() {
    SDL_Log("TEST BEGIN");
    System<Velocity> system;
    Id h0 = system.create();
    Id h1 = system.create();

    Velocity* v = system.get(h0);
    if (!v) {
        SDL_Log("ERROR!");
    }
    v->x = 10;
    v->y = 20;

    v = system.get(h1);
    if (v) {
        v->x = 30;
        v->y = 40;
    }

    v = system.get(h1);
    SDL_Log("v1: %f, %f", v->x, v->y);
    v = system.get(h0);
    SDL_Log("v0: %f, %f", v->x, v->y);

    system.free(h0);
    Id h2 = system.create();
    v = system.get(h2);
    SDL_Log("v2: %f, %f", v->x, v->y);

    SDL_Log("TEST END");
};
