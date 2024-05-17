#pragma once

#include <memory>

#include "gfx.hpp"
#include "input.hpp"
#include "node.hpp"

std::unique_ptr<World> createWorld(Size size,
                                   UpdateContext& updateContext,
                                   RenderContext& renderContext);

class App {
  public:
    App();
    ~App();

    void init();
    void event(const SDL_Event*);
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

  protected:
  private:
    void onResizeEvent(const SDL_Event* ev);
    void onKeyEvent(const SDL_Event* ev);
    void onMouseButtonEvent(const SDL_Event* ev);
    void onMouseMotionEvent(const SDL_Event* ev);

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    bool m_exit = false;
    bool m_error = false;
    bool m_needsRendering = true;
    Size m_size;
    UpdateContext m_updateContext;
    RenderContext m_renderContext;

    InputManager m_inputManager;

    std::unique_ptr<World> m_world;
};