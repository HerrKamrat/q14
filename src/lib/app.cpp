#include "app.hpp"

#include "misc.hpp"

class NullWorld : public World {
  public:
    void init(UpdateContext& updateContext, RenderContext& renderContext) override {};
    void update(UpdateContext& context) override {};
    void render(RenderContext& context) override {};
    bool isAnimating() const override {
        return false;
    }
    void resize(Size size) override {};
    void debug(Debugger& debug) override {};
};

App::App() : m_renderContext(nullptr), m_world(std::make_unique<NullWorld>()) {
}

App::~App() {
}

void App::init(AppConfig config) {
    SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow(config.name, config.width, config.height, flags);
    if (!window) {
        m_error = true;
        return;
    }

    if (config.width <= 0 || config.height <= 0) {
        auto display = SDL_GetDisplayForWindow(window);
        SDL_Rect bounds;
        if (!SDL_GetDisplayUsableBounds(display, &bounds)) {
            float s = SDL_GetWindowDisplayScale(window);
            float f = 1.0f / s;
            SDL_SetWindowSize(window, static_cast<int>(bounds.w * f),
                              static_cast<int>(bounds.h * f));
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_DestroyWindow(window);
        m_error = true;
        return;
    }

    SDL_ShowWindow(window);

    {
        auto name = SDL_GetRendererName(renderer);
        if (name) {
            SDL_Log("Renderer: %s", name);
        }
        int max_texture_size = static_cast<int>(SDL_GetNumberProperty(
            SDL_GetRendererProperties(renderer), SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0));
        SDL_Log("Max texture size: %d x %d", max_texture_size, max_texture_size);
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
    m_clearColor = config.clearColor;

    m_updateContext.setTicks(SDL_GetTicks());
    m_renderContext = {m_renderer};

    m_inputManager.init();

    m_debugger.init(window, renderer);
}

void App::event(const SDL_Event* event) {
    m_debugger.event(event);
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
        // TODO: is this correct behaviour?
        // return;
    }

    switch (event->type) {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
            onResizeEvent(event);
            break;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            onKeyEvent(event);
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

void App::iterate() {
    // preUpdate();
    if (m_worldToChangeTo) {
        m_world = std::move(m_worldToChangeTo);
    }

    update();
    // postUpdate();
    // preRender();
    render();
    // postRender();
};

void App::update() {
    const double updateRate = 1.0 / 60.0;
    const uint64_t updateTicks = static_cast<uint64_t>(updateRate * 1000);
    const int maxIterations = 5;

    m_debugger.preUpdate();
    auto currentTicks = SDL_GetTicks();
    auto lastTicks = m_updateContext.getTicks();
    auto delta = currentTicks - lastTicks;
    // TODO: fix this for real
    int iterations = static_cast<int>(delta / updateTicks);

    for (int i = std::min(iterations, maxIterations); i > 0; i--) {
        lastTicks = lastTicks + updateTicks;
        m_updateContext.setTicks(lastTicks);
        m_updateContext.setInputState(m_inputManager.getState(true));
        m_world->update(m_updateContext);
    }

    m_updateContext.setTicks(lastTicks + iterations * updateTicks);
    if (m_debugger.active()) {
        m_world->debug(m_debugger);
        m_renderContext.debug(m_debugger);
    }
    m_debugger.postUpdate(m_updateContext);
}

void App::render() {
    if (!m_needsRendering && !m_world->isAnimating()) {
        return;
    }
    m_needsRendering = false;
    m_renderContext.clear(m_clearColor);

    m_world->render(m_renderContext);

    if (m_renderContext.frameCount() % 2 == 0) {
        m_renderContext.setColor({255, 0, 0, 125});
        m_renderContext.drawRect({{0, 0}, m_size}, true);
        m_renderContext.setColor(Colors::WHITE);
    }

    // m_debugger.pushFrame();

    m_debugger.render();

    m_renderContext.present();
}

int App::status() {
    return m_error ? -1 : (m_exit ? 1 : 0);
}

void App::setWorld(std::unique_ptr<World> world) {
    m_world = std::move(world);
    if (m_world) {
        m_world->init(m_updateContext, m_renderContext);
        m_world->resize(m_size);
    }
}

void App::onResizeEvent(const SDL_Event* ev) {
    m_size = {(float)ev->window.data1, (float)ev->window.data2};
    SDL_Log("ResizeEvent: %d x %d", ev->window.data1, ev->window.data2);
    m_world->resize(m_size);
    // ResizeEvent event(ev);
    // m_world->onResizeEvent(event);
};

void App::onKeyEvent(const SDL_Event* ev) {
    KeyboardEvent event(ev);
    // m_world->onKeyboardEvent(event);
}

void App::onMouseButtonEvent(const SDL_Event* ev) {
    MouseButtonEvent event(ev);
    // m_world->onMouseButtonEvent(event);
}

void App::onMouseMotionEvent(const SDL_Event* ev) {
    MouseMotionEvent event(ev);
    // m_world->onMouseMotionEvent(event);
}
