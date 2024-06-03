#pragma once

#include <SDL.h>

struct AnalogInputValue {
    float value = 0;
    bool justTriggered = false;
};

struct DigitalInputValue {
    bool pressed = false;
    bool justTriggered = false;
};

struct InputState {
    AnalogInputValue up;
    AnalogInputValue down;
    AnalogInputValue left;
    AnalogInputValue right;

    DigitalInputValue primaryAction;
    DigitalInputValue secondaryAction;
};

class InputManager {
  public:
    bool init();

    bool handleEvent(const SDL_Event* event);

    InputState getState(bool clearNewFlags = false);

  protected:
  private:
    InputState m_state;
};
