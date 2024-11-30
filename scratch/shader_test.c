#include <bgame/scene.h>
#include <bgame/asset.h>
#include <bgame/asset/sprite.h>
#include <bgame/reloadable.h>
#include <cute_app.h>
#include <cute_draw.h>
#include <cute_file_system.h>
#include <cute_graphics.h>
#include <cute_sprite.h>
#include <cute_input.h>
#include <cimgui.h>

BGAME_VAR(bgame_asset_bundle_t*, bundle_shader_test) = NULL;
BGAME_VAR(CF_Sprite*, spr_shader_test) = NULL;
BGAME_VAR(CF_Shader, shd_glow) = { 0 };
BGAME_VAR(CF_Shader, shd_solid_color) = { 0 };
BGAME_VAR(CF_Color, shd_glow_color) = { 0.f, 0.f, 1.f, 1.0f };
BGAME_VAR(float, shd_glow_thickness) = 1.f;
BGAME_VAR(CF_Canvas, canvas_highlight) = { 0 };
BGAME_VAR(bool, draw_glow) = true;
BGAME_VAR(bool, draw_main) = true;
static bool shader_compiled = false;
static CF_Sprite spr_girl = { 0 };

#define STR(X) #X

// Safe compile function so we don't crash
static void
compile_shader(CF_Shader* shader, const char* name) {
	CF_Shader new_shader = cf_make_draw_shader(name);
	if (new_shader.id != 0) {
		if (shader->id != 0) {
			cf_destroy_shader(*shader);
		}
		*shader = new_shader;
	} else {
		shader_compiled = false;
	}
}

static void
init(int argc, const char* argv[]) {
	bgame_asset_begin_load(&bundle_shader_test);
	spr_shader_test = bgame_load_sprite(bundle_shader_test, "/assets/white-pawn.aseprite");
	bgame_asset_end_load(bundle_shader_test);

	shader_compiled = true;
	compile_shader(&shd_solid_color, "solid_color.shd");
	compile_shader(&shd_glow, "glow.shd");

	spr_girl = cf_make_demo_sprite();
	cf_sprite_play(&spr_girl, "spin");
	spr_girl.scale = (CF_V2){ 2.f, 2.f };

	if (canvas_highlight.id == 0) {
		int w, h;
		cf_app_get_size(&w, &h);
		CF_CanvasParams params = cf_canvas_defaults(w, h);
		params.target.wrap_u = CF_WRAP_MODE_CLAMP_TO_EDGE;
		params.target.wrap_v = CF_WRAP_MODE_CLAMP_TO_EDGE;
		canvas_highlight = cf_make_canvas(params);
	}
}

static void
cleanup(void) {
	bgame_asset_destroy_bundle(bundle_shader_test);
	if (canvas_highlight.id != 0) {
		cf_destroy_canvas(canvas_highlight);
	}

	cf_destroy_shader(shd_solid_color);
	cf_destroy_shader(shd_glow);
}

static void
draw_scene(void) {
	int w, h;
	cf_app_get_size(&w, &h);

	cf_draw_sprite(spr_shader_test);
	cf_draw_sprite(&spr_girl);
}

static void
update(void) {
	bgame_asset_check_bundle(bundle_shader_test);

	cf_app_update(NULL);
	cf_sprite_update(spr_shader_test);
	cf_sprite_update(&spr_girl);

	int w, h;
	cf_app_get_size(&w, &h);

	if (shader_compiled) {
		cf_clear_color(shd_glow_color.r, shd_glow_color.g, shd_glow_color.b, 0.f);
		cf_draw_push_antialias(false);
		{
			draw_scene();
		}
		cf_draw_pop_antialias();
		cf_render_to(canvas_highlight, true);
	}
	cf_clear_color(0.5f, 0.5f, 0.5f, 1.f);

	if (draw_glow && shader_compiled) {
		cf_draw_push_layer(0);
		cf_draw_push_shader(shd_glow);
		cf_draw_set_uniform_float("thickness", shd_glow_thickness);
		cf_draw_push_antialias(false);
		cf_draw_canvas(canvas_highlight, (CF_V2){ 0, 0 }, (CF_V2){ w, h });
		cf_draw_pop_antialias();
		cf_draw_pop_shader();
		cf_draw_pop_layer();
	}

	if (draw_main) {
		cf_draw_push_layer(1);
		draw_scene();
		cf_draw_pop_layer();
	}

	if (cf_mouse_just_pressed(CF_MOUSE_BUTTON_RIGHT)) {
		spr_girl.transform.p = cf_screen_to_world((CF_V2){cf_mouse_x(), cf_mouse_y()});
	}

	igBegin("Test", NULL, ImGuiWindowFlags_None);
		igCheckbox("Draw glow", &draw_glow);
		igCheckbox("Draw main", &draw_main);
		igColorPicker3("Glow color", (float*)&shd_glow_color.r, ImGuiColorEditFlags_None);
		igSliderFloat("Thickness", &shd_glow_thickness, 0.f, 12.f, "%f", ImGuiSliderFlags_None);
	igEnd();

	cf_app_draw_onto_screen(true);
}

BGAME_SCENE(shader_test) = {
	.init = init,
	.cleanup = cleanup,
	.update = update,
};
