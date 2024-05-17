#include "app.hpp"

#include "misc.hpp"

App::App() : m_renderContext(nullptr), m_world(std::make_unique<World>()) {
}

App::~App() {
}

void App::init() {
    SDL_Window* window = SDL_CreateWindow(version(), 640, 480, SDL_WINDOW_RESIZABLE);
    if (!window) {
        m_error = true;
        return;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL, 0);
    if (!renderer) {
        SDL_DestroyWindow(window);
        m_error = true;
        return;
    }

    SDL_ShowWindow(window);

    {
        SDL_RendererInfo info;
        SDL_GetRendererInfo(renderer, &info);
        SDL_Log("Renderer: %s", info.name);
        SDL_Log("Max texture size: %d x %d", info.max_texture_width, info.max_texture_height);
    }

    int width, height, bbwidth, bbheight;
    SDL_GetWindowSize(window, &width, &height);
    SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
    SDL_Log("Window size: %ix%i", width, height);
    SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);
    if (width != bbwidth) {
        SDL_Log("This is a highdpi environment.");
    }

    m_window = window;
    m_renderer = renderer;
    m_size = {(float)bbwidth, (float)bbheight};

    m_renderContext = {m_renderer};

    m_inputManager.init();
}

void App::event(const SDL_Event* event) {
#ifdef ANDROID
    if (event->type == SDL_EVENT_KEY_UP && event->key.keysym.scancode == SDL_SCANCODE_AC_BACK) {
        SDL_MinimizeWindow(m_window);
        return;
    }
#endif
    m_needsRendering = true;
    if (event->type == SDL_EVENT_QUIT) {
        m_exit = true;
    }
#ifndef EMSCRIPTEN
    else if (event->type == SDL_EVENT_KEY_UP && event->key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
        m_exit = true;
    }
#endif

    if (m_inputManager.handleEvent(event)) {
        return;
    }

    switch (event->type) {
        case SDL_EVENT_WINDOW_RESIZED: {
            onResizeEvent(event);
            break;
        }

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            onKeyEvent(event);
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            SDL_Log("gamepad axis - %d: %d", event->gaxis.axis, event->gaxis.value);
            break;
        }
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            SDL_Log("gamepad button - button: %d", event->gbutton.button);
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            onMouseMotionEvent(event);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            onMouseButtonEvent(event);
            break;
        }
    };
}

void App::update() {
    m_updateContext.setTicks(SDL_GetTicks());
    m_world->update(m_updateContext);
}

void App::render() {
    if (!m_needsRendering && !m_world->isAnimating()) {
        return;
    }
    m_needsRendering = false;
    m_renderContext.clear();

    m_world->render(m_renderContext);

    if (m_renderContext.frameCount() % 2 == 0) {
        m_renderContext.setColor(Colors::RED);
        m_renderContext.drawRect({{0, 0}, m_size}, true);
        m_renderContext.setColor(Colors::WHITE);
    }

    m_renderContext.present();
}

int App::status() {
    return m_error ? -1 : (m_exit ? 1 : 0);
}

void App::setWorld(std::unique_ptr<World> world) {
    m_world = std::move(world);
    if (m_world) {
        m_world->init(m_updateContext, m_renderContext);
    }
}

void App::onResizeEvent(const SDL_Event* ev) {
    m_size = {(float)ev->window.data1, (float)ev->window.data2};
    SDL_Log("ResizeEvent: %d x %d", ev->window.data1, ev->window.data2);
    ResizeEvent event(ev);
    m_world->onResizeEvent(event);
};

void App::onKeyEvent(const SDL_Event* ev) {
    KeyboardEvent event(ev);
    m_world->onKeyboardEvent(event);
}

void App::onMouseButtonEvent(const SDL_Event* ev) {
    MouseButtonEvent event(ev);
    m_world->onMouseButtonEvent(event);
}

void App::onMouseMotionEvent(const SDL_Event* ev) {
    MouseMotionEvent event(ev);
    m_world->onMouseMotionEvent(event);
}
