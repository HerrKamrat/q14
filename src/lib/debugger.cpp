
#include "debugger.hpp"

namespace {

constexpr const char* WINDOW_NAME = "DEBUG";
constexpr int ROW_HEIGHT = 15;

};  // namespace

// #pragma warning(push, 0)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#define NK_INCLUDE_FIXED_TYPES
// #define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#define NK_IMPLEMENTATION
#include "third_party/nuklear.h"

#define NK_SDL_RENDERER_IMPLEMENTATION
#define NK_SDL_CLAMP_CLIP_RECT
#include "third_party/nuklear_sdl_renderer.h"

#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

extern "C" {
nk_bool nkx_button_text(struct nk_context* ctx, const char* title, bool enabled) {
    if (!ctx)
        return 0;
    if (!enabled) {
        nk_widget_disable_begin(ctx);
    }
    auto r = nk_button_text(ctx, title, static_cast<int>(strlen(title)));
    if (!enabled) {
        nk_widget_disable_end(ctx);
    }
    return r;
}

nk_bool nkx_button_symbol(struct nk_context* ctx, enum nk_symbol_type symbol, bool enabled) {
    if (!ctx)
        return 0;
    if (!enabled) {
        nk_widget_disable_begin(ctx);
    }
    auto r = nk_button_symbol(ctx, symbol);
    if (!enabled) {
        nk_widget_disable_end(ctx);
    }
    return r;
}
}

void Debugger::init(SDL_Window* window, SDL_Renderer* renderer) {
    float scale = SDL_GetWindowDisplayScale(window);
    float font_scale = scale;

    // TODO: fix nk_sdl_* to not use a global context
    nk_context* ctx = nk_sdl_init(window, renderer, scale);
    m_ctx = {ctx, nk_free};
    // Font
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
    // Style
    {
        struct nk_color table[NK_COLOR_COUNT];
        table[NK_COLOR_TEXT] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(30, 33, 40, 215);
        table[NK_COLOR_HEADER] = nk_rgba(181, 45, 69, 220);
        table[NK_COLOR_BORDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(190, 50, 70, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(195, 55, 75, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 60, 60, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SELECT] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(186, 50, 74, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(191, 55, 79, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_EDIT] = nk_rgba(51, 55, 67, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_COMBO] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(170, 40, 60, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 33, 40, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(181, 45, 69, 220);

        nk_style_from_table(ctx, table);
    }

    // Begin input for the first frame
    nk_input_begin(ctx);
}

void Debugger::event(const SDL_Event* event) {
    if (event->type == SDL_EVENT_KEY_UP && event->key.keysym.scancode == SDL_SCANCODE_F1) {
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

    m_windowShown =
        m_windowShown &&
        nk_begin(ctx, WINDOW_NAME, nk_rect(0, 0, 300, static_cast<float>(m_windowHeight)),
                 NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE);
}

void Debugger::postUpdate(const UpdateContext& context) {
    auto ctx = m_ctx.get();

    if (m_windowShown) {
        {
            auto state = context.getInputState();
            if (nk_tree_push(ctx, NK_TREE_TAB, "INPUT", NK_MAXIMIZED)) {
                nk_layout_row_static(ctx, 20, 20, 2);
                nk_spacer(ctx);
                nkx_button_symbol(ctx, NK_SYMBOL_TRIANGLE_UP, state.up.active());

                nk_layout_row_static(ctx, 20, 20, 6);
                nkx_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT, state.left.active());
                nkx_button_symbol(ctx, NK_SYMBOL_TRIANGLE_DOWN, state.down.active());
                nkx_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT, state.right.active());
                nk_spacer(ctx);
                nkx_button_text(ctx, "I", state.primaryAction.active());
                nkx_button_text(ctx, "2", state.secondaryAction.active());

                nk_tree_pop(ctx);
            }
        }
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
                                   static_cast<int>(m_logs.size()))) {
                nk_layout_row_dynamic(ctx, ROW_HEIGHT, 1);
                for (int i = 0; i < view.count; ++i) {
                    nk_label(ctx, m_logs.at(view.begin + i).data(), NK_TEXT_LEFT);
                }
                nk_list_view_end(&view);
            }
            nk_tree_pop(ctx);
        }
        nk_end(ctx);
    }

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
