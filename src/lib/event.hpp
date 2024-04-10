#pragma once
#include <SDL.h>

#include "math.hpp"

class Event {
  public:
    Event(const SDL_Event* event) : m_event(event){};

    const SDL_Event* rawEvent() const {
        return m_event;
    }

    void stopPropagation() {
        m_stopPropagation = true;
    };

    bool isPropagationStopped() const {
        return m_stopPropagation;
    }

  private:
    const SDL_Event* m_event;
    bool m_stopPropagation = false;
};

class ResizeEvent : public Event {
  public:
    using Event::Event;
    Size size() {
        return {(float)event().data1, (float)event().data2};
    }

  protected:
  private:
    const SDL_WindowEvent& event() const {
        return rawEvent()->window;
    }
};

class KeyboardEvent : public Event {
  public:
    using Event::Event;

  protected:
  private:
    const SDL_KeyboardEvent& event() const {
        return rawEvent()->key;
    }
};

class MouseButtonEvent : public Event {
  public:
    using Event::Event;
    bool pressed() const {
        return event().state == SDL_PRESSED;
    }

    bool released() const {
        return !pressed();
    }

    Vec2 position() const {
        return {event().x, event().y};
    }

  protected:
  private:
    const SDL_MouseButtonEvent& event() const {
        return rawEvent()->button;
    }
};

class MouseMotionEvent : public Event {
  public:
    using Event::Event;
    Vec2 position() {
        return {event().x, event().y};
    }

    Vec2 delta() {
        return {event().xrel, event().yrel};
    }

  protected:
  private:
    const SDL_MouseMotionEvent& event() const {
        return rawEvent()->motion;
    }
};

class EventListener {
  public:
    virtual void onResizeEvent(ResizeEvent& resize){};
    virtual void onKeyboardEvent(KeyboardEvent& event){};
    virtual void onMouseButtonEvent(MouseButtonEvent& event){};
    virtual void onMouseMotionEvent(MouseMotionEvent& event){};
};