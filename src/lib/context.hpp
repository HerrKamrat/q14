#pragma once

#include <cstdint>
#include "input.hpp"

class UpdateContext {
  public:
    void setTicks(uint64_t ticks) {
        auto prev = m_ticks;
        m_ticks = ticks;
        m_ticksDelta = ticks - prev;
    };

    uint64_t getTicks() const {
        return m_ticks;
    }

    float getDeltaTime() {
        return m_ticksDelta / 1000.0f;
    }

    void setInputState(InputState inputState) {
        m_inputState = inputState;
    }

    const InputState& getInputState() const {
        return m_inputState;
    }

  protected:
  private:
    uint64_t m_ticks;
    uint64_t m_ticksDelta;

    InputState m_inputState;
};
