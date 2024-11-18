#include <bgame/reloadable.h>
#include <bgame/allocator.h>
#include <bgame/scene.h>
#include <bgame/ui.h>
#include <clay.h>
#include <cute_app.h>
#include <cute_draw.h>
#include <cute_sprite.h>
#include <bhash.h>

BGAME_DECLARE_TRACKED_ALLOCATOR(main_scene_alloc)

typedef BHASH_TABLE(const char*, CF_Sprite) sprite_table_t;

BGAME_VAR(CF_Sprite, sprite) = { 0 };
BGAME_VAR(sprite_table_t, sprite_instances) = { 0 };

static void
init(int argc, const char** argv) {
	if (!sprite.name) {
		sprite = cf_make_demo_sprite();
	}

	bhash_config_t config = bhash_config_default();
	config.memctx = &main_scene_alloc.allocator;
	if (sprite_instances.keys == NULL) {
		bhash_init(&sprite_instances, config);
	}
	sprite_instances.base.memctx = &main_scene_alloc.allocator;
	sprite_instances.base.hash = config.hash;
	sprite_instances.base.eq = config.eq;

	Clay_SetDebugModeEnabled(true);
}

static void
cleanup(void) {
	bhash_cleanup(&sprite_instances);
}

static void
fixed_update(void* udata) {
	(void)udata;
}

static void
update(void) {
	cf_app_update(fixed_update);
	cf_clear_color(0.5f, 0.5f, 0.5f, 1.f);

	cf_push_font("Calibri");
	bgame_ui_begin();

	Clay_Color root_bg = { 127, 128, 128, 255 };
	Clay_Color sidebar_bg = { 100, 100, 100, 255 };
	Clay_Color text_color = { 255, 255, 255, 255 };

	CLAY(
		CLAY_ID("root"),
		CLAY_LAYOUT({
			.sizing = {
				.width = CLAY_SIZING_GROW({ 0 }),
				.height = CLAY_SIZING_GROW({ 0 })
			},
			.padding = { 16, 16 },
			.childGap = 16
		}),
		CLAY_RECTANGLE({ .color = root_bg })
	) {

		CLAY(
			CLAY_ID("sidebar"),
			CLAY_LAYOUT({
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				.sizing = {
					.width = CLAY_SIZING_FIXED(300),
					.height = CLAY_SIZING_GROW({ 0 })
				},
				.padding = {16, 16},
				.childGap = 16
			}),
			CLAY_RECTANGLE({
				.color = sidebar_bg,
				.cornerRadius.topLeft = 10.f,
			})
		) {
			CLAY_TEXT(
				CLAY_STRING("Side bar with a long affffff titlef what is this clipping?"),
				CLAY_TEXT_CONFIG({
					.fontSize = 24,
					.textColor = text_color,
					.fontName = "Calibri"
				})
			);
		}

		CLAY(
			CLAY_ID("content"),
			CLAY_LAYOUT({
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				.sizing = {
					.width = CLAY_SIZING_GROW({ 0 }),
					.height = CLAY_SIZING_GROW({ 0 })
				},
				.padding = {16, 16},
				.childGap = 16
			}),
			CLAY_RECTANGLE({
				.color = sidebar_bg,
				.cornerRadius.topLeft = 10.f,
			})
		) {
			CLAY_TEXT(
				CLAY_STRING("<shake>Content</shake>"),
				CLAY_TEXT_CONFIG({ .fontSize = 24, .textColor = text_color })
			);

			CLAY(
				CLAY_ID("sprite_container"),
				CLAY_LAYOUT({
					.layoutDirection = CLAY_LEFT_TO_RIGHT,
					.sizing = {
						.width = CLAY_SIZING_GROW({ 0 }),
						.height = CLAY_SIZING_GROW({ 0 })
					},
					.childAlignment.x = CLAY_ALIGN_X_LEFT,
					.childGap = 10,
				})
			) {
				// Create a separate instance for each animation
				// This has to be done in a separate pass since the sprite address
				// can change.
				for (int i = 0; i < hsize(sprite.animations); ++i) {
					const CF_Animation* animation = sprite.animations[i];
					const char* anim_name = animation->name;

					if (!bhash_is_valid(bhash_find(&sprite_instances, anim_name))) {
						bhash_put(&sprite_instances, anim_name, sprite);
						bhash_index_t index = bhash_find(&sprite_instances, anim_name);
						cf_sprite_play(&sprite_instances.values[index], anim_name);
					}
				}

				for (int i = 0; i < bhash_len(&sprite_instances); ++i) {
					CLAY(
						CLAY_IDI_LOCAL("Animation", i),
						CLAY_LAYOUT({
							.layoutDirection = CLAY_TOP_TO_BOTTOM,
							.childAlignment.x = CLAY_ALIGN_X_CENTER,
							.sizing = {
								.width = CLAY_SIZING_FIT({ 0 }),
								.height = CLAY_SIZING_FIT({ 0 }),
							},
							.padding = { 5, 5 },
							}),
						CLAY_BORDER_OUTSIDE({
							.color = bgame_ui_color(cf_color_white()),
							.width = 1,
						})
					) {
						CF_Sprite* instance = &sprite_instances.values[i];
						cf_sprite_update(instance);

						CLAY(
							CLAY_ID_LOCAL("sprite"),
							CLAY_LAYOUT({
								.sizing = {
									.width = CLAY_SIZING_FIXED(instance->w),
									.height = CLAY_SIZING_FIXED(instance->h),
								}
							}),
							CLAY_IMAGE({
								.imageData = instance,
								.sourceDimensions = {
									.width = instance->w,
									.height = instance->h,
								},
							})
						) {
						}

						CLAY_TEXT(
							((Clay_String){
								.length = strlen(instance->animation->name),
								.chars = instance->animation->name,
							}),
							CLAY_TEXT_CONFIG({
								.fontSize = 15,
								.textColor = text_color
							})
						);
					}
				}
			}
		}
	}

	bgame_ui_end();
	cf_pop_font();

	cf_app_draw_onto_screen(true);
}

BGAME_SCENE(main_scene) = {
	.init = init,
	.cleanup = cleanup,
	.update = update,
};
