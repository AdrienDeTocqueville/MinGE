#include "Profiler/profiler.h"

#define NK_IMPLEMENTATION
#include "UI/ui.h"

#include "Graphics/GLDriver.h"
#include "UI/nuklear_sfml_gl3.h"

#include "IO/Input.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

struct nk_context *UI::ctx;

void UI::init()
{
	ctx = nk_sfml_init(Input::window());

	/* Load Fonts: if none of these are loaded a default font will be used  */
	/* Load Cursor: if you uncomment cursor loading please hide the cursor */
	struct nk_font_atlas *atlas;
	nk_sfml_font_stash_begin(&atlas);
	/*struct nk_font *droid = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14, 0);*/
	/*struct nk_font *roboto = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 14, 0);*/
	/*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
	/*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
	/*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
	/*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
	nk_sfml_font_stash_end();

	/*nk_style_load_all_cursors(ctx, atlas->cursors);*/
	/*nk_style_set_font(ctx, &droid->handle);*/
}

void UI::destroy()
{
	nk_sfml_shutdown();
}

void UI::send_inputs()
{
	ivec2 mouse_pos = Input::mouse_position(false);
	int wheel_scroll = Input::wheel_scroll();

	nk_input_begin(ctx);
	nk_input_motion(ctx, mouse_pos.x, mouse_pos.y);
        nk_input_scroll(ctx, nk_vec2(0, wheel_scroll));
	nk_input_button(ctx, NK_BUTTON_LEFT, mouse_pos.x, mouse_pos.y, Input::button_down(sf::Mouse::Button::Left));
	nk_input_button(ctx, NK_BUTTON_MIDDLE, mouse_pos.x, mouse_pos.y, Input::button_down(sf::Mouse::Button::Middle));
	nk_input_button(ctx, NK_BUTTON_RIGHT, mouse_pos.x, mouse_pos.y, Input::button_down(sf::Mouse::Button::Right));
	nk_input_end(ctx);
}

void UI::flush()
{
	if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
		NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		enum { EASY, HARD };
		static int op = EASY;
		static int property = 20;
		static struct nk_colorf bg;

		nk_layout_row_static(ctx, 30, 80, 1);
		if (nk_button_label(ctx, "button"))
			fprintf(stdout, "button pressed\n");

		nk_layout_row_dynamic(ctx, 30, 2);
		if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
		if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

		nk_layout_row_dynamic(ctx, 20, 1);
		nk_label(ctx, "background:", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx), 400))) {
			nk_layout_row_dynamic(ctx, 120, 1);
			bg = nk_color_picker(ctx, bg, NK_RGBA);
			nk_layout_row_dynamic(ctx, 25, 1);
			bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
			bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
			bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
			bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
			nk_combo_end(ctx);
		}
	}
	nk_end(ctx);

	MICROPROFILE_SCOPEI("RENDER_ENGINE", "ui");
	MICROPROFILE_SCOPEGPUI("nuklear", -1);

	nk_sfml_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
}
