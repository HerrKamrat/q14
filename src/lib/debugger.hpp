#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "node.hpp"

struct nk_context;

class Debugger {
  public:
    void init(SDL_Window* win, SDL_Renderer* renderer);
    void event(const SDL_Event* event);
    void update();
    void preUpdate();
    void postUpdate(const UpdateContext& context);
    void render();

    void log(const char* log);

  private:
    std::unique_ptr<nk_context, void (*)(nk_context*)> m_ctx{nullptr, nullptr};
    std::vector<std::tuple<std::string, std::string>> m_values;
    std::vector<std::string> m_logs;
    bool m_windowShown = true;
    bool m_toggleWindow = false;
    int m_windowHeight = 0;
};
