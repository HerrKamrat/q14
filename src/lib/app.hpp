#pragma once

#include <memory>

#include "debugger.hpp"
#include "gfx.hpp"
#include "input.hpp"
#include "world.hpp"

struct AppConfig {
    const char* name{nullptr};
    int width{-1};
    int height{-1};
    Color clearColor{Colors::WHITE};
};

class App {
  public:
    App();
    ~App();

    void init(AppConfig config = AppConfig());
    void event(const SDL_Event*);
    void iterate();
    void update();
    void render();

    int status();

    void setWorld(std::unique_ptr<World> world);
    World* getWorld() {
        return m_world.get();
    };

    SDL_Window* window() const {
        return m_window;
    };
    SDL_Renderer* renderer() const {
        return m_renderer;
    };
    Debugger* debugger() {
        return &m_debugger;
    }

  protected:
  private:
    void onResizeEvent(const SDL_Event* ev);
    void onKeyEvent(const SDL_Event* ev);
    void onMouseButtonEvent(const SDL_Event* ev);
    void onMouseMotionEvent(const SDL_Event* ev);

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool m_exit = false;
    bool m_error = false;
    bool m_needsRendering = true;
    Size m_size;
    Color m_clearColor = Colors::BLACK;
    UpdateContext m_updateContext;
    RenderContext m_renderContext;

    InputManager m_inputManager;

    std::unique_ptr<World> m_worldToChangeTo;
    std::unique_ptr<World> m_world;

    Debugger m_debugger;
};