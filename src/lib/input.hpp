#pragma once

#include <SDL.h>

class InputManager {
  public:
    bool init();

    bool handleEvent(const SDL_Event* event);

  protected:
  private:
};
