
#include "debugger.hpp"

// #pragma warning(push, 0)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#define NK_INCLUDE_FIXED_TYPES
// #define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

namespace {
constexpr const char* WINDOW_NAME = "INFO";
constexpr int ROW_HEIGHT = 15;
};  // namespace

#define NK_IMPLEMENTATION
#include "third_party/nuklear.h"

#define NK_SDL_RENDERER_IMPLEMENTATION
#define NK_SDL_CLAMP_CLIP_RECT ;
#include "third_party/nuklear_sdl_renderer.h"

#pragma GCC diagnostic pop

void Debugger::init(SDL_Window* window, SDL_Renderer* renderer) {
    float scale = SDL_GetWindowDisplayScale(window);
    float font_scale = scale;

    // TODO: fix nk_sdl_* to not use a global context
    nk_context* ctx = nk_sdl_init(window, renderer, scale);
    m_ctx = {ctx, nk_free};
    {
        struct nk_font_atlas* atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font* font;

        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);

        nk_sdl_font_stash_end();

        font->handle.height /= font_scale;
        // nk_style_load_all_cursors(ctx, atlas->cursors);
        nk_style_set_font(ctx, &font->handle);
    }

    // Begin input for the first frame
    nk_input_begin(ctx);
}

void Debugger::event(const SDL_Event* event) {
    if (event->type == SDL_EVENT_KEY_UP && event->key.keysym.scancode == SDL_SCANCODE_Q) {
        m_toggleWindow = true;
    } else if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        m_windowHeight = event->window.data2;
    }

    nk_sdl_handle_event(event);
}

void Debugger::preUpdate() {
    auto ctx = m_ctx.get();
    nk_input_end(ctx);

    if (m_toggleWindow) {
        m_toggleWindow = false;
        m_windowShown = !m_windowShown;
    }

    m_windowShown = m_windowShown && nk_begin(ctx, WINDOW_NAME, nk_rect(0, 0, 300, m_windowHeight),
                                              NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE);
}

void Debugger::postUpdate() {
    auto ctx = m_ctx.get();

    if (m_windowShown) {
        if (nk_tree_push(ctx, NK_TREE_TAB, "VALUES", NK_MAXIMIZED)) {
            for (auto& entry : m_values) {
                nk_layout_row_dynamic(ctx, ROW_HEIGHT, 2);
                nk_label(ctx, std::get<0>(entry).data(), NK_TEXT_LEFT);
                nk_label(ctx, std::get<1>(entry).data(), NK_TEXT_LEFT);
            }
            nk_tree_pop(ctx);
        }
        if (nk_tree_push(ctx, NK_TREE_TAB, "LOG", NK_MAXIMIZED)) {
            struct nk_list_view view;
            nk_layout_row_dynamic(ctx, ROW_HEIGHT * 10 * 1.2, 1);
            if (nk_list_view_begin(ctx, &view, "test", NK_WINDOW_BORDER, ROW_HEIGHT,
                                   m_logs.size())) {
                nk_layout_row_dynamic(ctx, ROW_HEIGHT, 1);
                for (int i = 0; i < view.count; ++i) {
                    nk_label(ctx, m_logs.at(view.begin + i).data(), NK_TEXT_LEFT);
                }
                nk_list_view_end(&view);
            }
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);

    // NOTE: Move this to nuklear_sdl_renderer
    if (ctx->text_edit.active) {
        if (!SDL_TextInputActive()) {
            SDL_StartTextInput();
        }
    } else {
        if (SDL_TextInputActive()) {
            SDL_StopTextInput();
        }
    }
    nk_input_begin(ctx);
    m_values.clear();
}

void Debugger::render() {
    nk_sdl_render(NK_ANTI_ALIASING_ON);
}

void Debugger::log(const char* log) {
    if (m_logs.size() >= 10) {
        m_logs.erase(m_logs.begin());
    }
    m_logs.push_back(log);
}

void Debugger::value(const char* key, const char* value) {
    m_values.push_back(std::make_tuple(key, value));
}
