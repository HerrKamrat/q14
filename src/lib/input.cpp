#include "input.hpp"

#include <SDL.h>

// https://github.com/libsdl-org/SDL/blob/main/docs/README-migration.md#sdl_gamecontrollerh
bool InputManager::init() {
    int count = 0;
    auto gamepads = SDL_GetGamepads(&count);

    for (int i = 0; i < count; i++) {
        auto gamepad = gamepads[i];
        SDL_OpenGamepad(gamepad);
    }

    return true;
}

bool InputManager::handleEvent(const SDL_Event* event) {
    switch (event->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            // SDL_Log("key: %d %d", event->key.keysym.scancode, event->key.state);
            switch (event->key.keysym.scancode) {
                case SDL_SCANCODE_W:
                    m_state.up.value = event->key.state;
                    break;
                case SDL_SCANCODE_A:
                    m_state.left.value = event->key.state;
                    break;
                case SDL_SCANCODE_S:
                    m_state.down.value = event->key.state;
                    break;
                case SDL_SCANCODE_D:
                    m_state.right.value = event->key.state;
                    break;
                case SDL_SCANCODE_K:
                    m_state.primaryAction.value = event->key.state;
                    break;
                case SDL_SCANCODE_L:
                    m_state.secondaryAction.value = event->key.state;
                    break;
                default:
                    break;
            };
            break;
        }
        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            SDL_Log("Gamepad axis motion, axis: %d, value: %d", event->gaxis.axis,
                    event->gaxis.value);
            break;
        }
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            SDL_Log("Gamepad button %s, button: %d",
                    event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ? "down" : "up",
                    event->gbutton.button);
            break;
        }
        case SDL_EVENT_GAMEPAD_ADDED: {
            SDL_OpenGamepad(event->gdevice.which);
            break;
        }
        default:
            return false;
    }
    return true;
}

InputState InputManager::getState(bool clearNewFlags) {
    InputState state = m_state;
    if (clearNewFlags) {
        // TODO
    }
    return state;
}
