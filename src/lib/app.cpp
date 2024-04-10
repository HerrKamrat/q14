#include "app.hpp"

App::App() : m_renderContext(nullptr) {
}

App::~App() {
}

void App::init() {
    SDL_Window* window = SDL_CreateWindow("tmp", 640, 480, SDL_WINDOW_RESIZABLE);
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

    m_world = createWorld(m_size, m_updateContext, m_renderContext);
}

void App::event(const SDL_Event* event) {
    m_needsRendering = true;
    if (event->type == SDL_EVENT_QUIT) {
        m_exit = true;
    }
#ifndef EMSCRIPTEN
    else if (event->type == SDL_EVENT_KEY_UP && event->key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
        m_exit = true;
    }
#endif

    switch (event->type) {
        case SDL_EVENT_WINDOW_RESIZED: {
            onResizeEvent(event->window);
            break;
        }

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            onKeyEvent(event->key);
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            onMouseMotionEvent(event->motion);
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            onMouseButtonEvent(event->button);
            break;
        }
    };
}

void App::update() {
    m_updateContext.setTicks(SDL_GetTicks());
    m_world.update(m_updateContext);
}

void App::render() {
    if (!m_needsRendering) {
        return;
    }
    m_needsRendering = false;
    m_renderContext.clear();

    m_world.render(m_renderContext);

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

void App::onResizeEvent(const SDL_WindowEvent& ev) {
    m_size = {(float)ev.data1, (float)ev.data2};
}

void App::onKeyEvent(const SDL_KeyboardEvent& ev) {
    bool down = ev.state == SDL_PRESSED;
    SDL_Log("KeyboardButton%s: %d", down ? "Down" : "Up", ev.keysym.scancode);
}

void App::onMouseButtonEvent(const SDL_MouseButtonEvent& ev) {
    bool down = ev.state == SDL_PRESSED;
    SDL_Log("MouseButton%s: %d", down ? "Down" : "Up", ev.button);
}

void App::onMouseMotionEvent(const SDL_MouseMotionEvent& ev) {
    SDL_Log("MouseMotion: %fx%f", ev.x, ev.y);
}
