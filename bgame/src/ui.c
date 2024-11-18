#define CLAY_IMPLEMENTATION
#include <bgame/reloadable.h>
#include <bgame/allocator.h>
#include <bgame/ui.h>
#include <bgame/asset/9patch.h>
#include <cute_app.h>
#include <cute_input.h>
#include <cute_draw.h>

typedef struct {
	Clay_Arena clay_arena;
} bgame_ui_ctx_t;

static bool bgame_ui_need_init = true;

BGAME_VAR(bool, bgame_ui_created) = false;
BGAME_VAR(bgame_ui_ctx_t, bgame_ui_ctx) = { 0 };
BGAME_DECLARE_TRACKED_ALLOCATOR(bgame_ui)

static Clay_Dimensions
bgame_ui_measure_text(Clay_String* text, Clay_TextElementConfig* config) {
	cf_push_font_size(config->fontSize);
	if (config->fontName) {
		cf_push_font(config->fontName);
	}
	CF_V2 size = cf_text_size(text->chars, text->length);
	if (config->fontName) {
		cf_pop_font();
	}
	cf_pop_font_size(config->fontSize);
	return (Clay_Dimensions){
		.width = size.x,
		.height = size.y,
	};
}

static inline void
bgame_ui_init(void) {
	if (!bgame_ui_need_init) { return; }

	if (!bgame_ui_created) {
		size_t mem_size = Clay_MinMemorySize();
		void* memory = bgame_malloc(mem_size, bgame_ui);
		bgame_ui_ctx.clay_arena = Clay_CreateArenaWithCapacityAndMemory(mem_size, memory);

		bgame_ui_created = true;
	}

	int width, height;
	cf_app_get_size(&width, &height);

	Clay_Initialize(bgame_ui_ctx.clay_arena, (Clay_Dimensions){
		.width = width,
		.height = height,
	});
	Clay_SetMeasureTextFunction(bgame_ui_measure_text);

	bgame_ui_need_init = false;
}

static inline CF_Aabb
bgame_ui_aabb(Clay_BoundingBox bbox) {
	return (CF_Aabb){
		.min = {
			.x = bbox.x,
			.y = -(bbox.y + bbox.height),
		},
		.max = {
			.x = bbox.x + bbox.width,
			.y = -bbox.y,
		},
	};
}

static inline void
bgame_ui_push_color(Clay_Color color) {
	cf_draw_push_color((CF_Color){
		.a = color.a / 255.f,
		.r = color.r / 255.f,
		.g = color.g / 255.f,
		.b = color.b / 255.f,
	});
}

void
bgame_ui_begin(void) {
	bgame_ui_init();

	int w, h;
	cf_app_get_size(&w, &h);
	Clay_SetLayoutDimensions((Clay_Dimensions){ w, h });

	Clay_SetPointerState(
		(Clay_Vector2){ cf_mouse_x(), cf_mouse_y() },
		cf_mouse_down(CF_MOUSE_BUTTON_LEFT)
	);
	Clay_UpdateScrollContainers(
		true,
		(Clay_Vector2){ 0.f, cf_mouse_wheel_motion() },
		CF_DELTA_TIME
	);

	Clay_BeginLayout();
}

void
bgame_ui_end(void) {
	Clay_RenderCommandArray cmds = Clay_EndLayout();

	int w, h;
	cf_app_get_size(&w, &h);

	float half_width = w * 0.5f;
	float half_height = h * 0.5f;

	cf_draw_push();
	cf_draw_translate(-half_width, half_height);
	for (uint32_t i = 0; i < cmds.length; ++i) {
		Clay_RenderCommand cmd = cmds.internalArray[i];
		switch (cmd.commandType) {
			case CLAY_RENDER_COMMAND_TYPE_NONE:
				break;
			case CLAY_RENDER_COMMAND_TYPE_RECTANGLE:
				if (cmd.config.rectangleElementConfig->nine_patch == NULL) {
					bgame_ui_push_color(cmd.config.rectangleElementConfig->color);
					cf_draw_box_rounded_fill(
						bgame_ui_aabb(cmd.boundingBox),
						cmd.config.rectangleElementConfig->cornerRadius.topLeft
					);
					cf_draw_pop_color();
				} else {
					bgame_draw_9patch(
						cmd.config.rectangleElementConfig->nine_patch,
						bgame_ui_aabb(cmd.boundingBox)
					);
				}
				break;
			case CLAY_RENDER_COMMAND_TYPE_BORDER:
				bgame_ui_push_color(cmd.config.borderElementConfig->left.color);
				// TODO: Different borders
				cf_draw_box_rounded(
					bgame_ui_aabb(cmd.boundingBox),
					cmd.config.borderElementConfig->top.width * 0.25f,
					cmd.config.borderElementConfig->cornerRadius.topLeft * 0.25f
				);
				cf_draw_pop_color();
				break;
			case CLAY_RENDER_COMMAND_TYPE_TEXT:
				cf_push_font_size(cmd.config.textElementConfig->fontSize);
				bgame_ui_push_color(cmd.config.textElementConfig->textColor);
				cf_draw_text(
					cmd.text.chars,
					(CF_V2){ cmd.boundingBox.x, -cmd.boundingBox.y },
					cmd.text.length
				);
				cf_draw_pop_color();
				cf_pop_font_size();
				break;
			case CLAY_RENDER_COMMAND_TYPE_IMAGE:
				{
					CF_Sprite* sprite = cmd.config.imageElementConfig->imageData;
					// TODO: clip or scale?
					CF_V2 pivot = sprite->pivots[sprite->frame_index];
					sprite->transform.p.x = cmd.boundingBox.x + sprite->w * 0.5f + pivot.x;
					sprite->transform.p.y = -cmd.boundingBox.y - sprite->h * 0.5f - pivot.y;
					cf_draw_sprite(sprite);
				}
				break;
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
				cf_draw_push_scissor((CF_Rect){
					.x = cmd.boundingBox.x,
					.y = cmd.boundingBox.y,
					.w = cmd.boundingBox.width,
					.h = cmd.boundingBox.height,
				});
				break;
			case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
				cf_draw_pop_scissor();
				break;
			case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
				break;
		}
	}
	cf_draw_pop();
}
