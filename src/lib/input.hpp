#pragma once

#include <SDL3/SDL.h>

struct AnalogInputValue {
    float value = 0;
    // bool justTriggered = false;
    inline bool active() const {
        return value != 0.0f;
    }
};

// struct DigitalInputValue {
//     bool pressed = false;
//     // bool justTriggered = false;
//     inline bool active() const {
//         return pressed;
//     }
// };

struct InputState {
    AnalogInputValue up;
    AnalogInputValue down;
    AnalogInputValue left;
    AnalogInputValue right;

    AnalogInputValue primaryAction;
    AnalogInputValue secondaryAction;
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
