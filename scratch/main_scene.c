#include <bgame/reloadable.h>
#include <bgame/allocator.h>
#include <bgame/allocator/tracked.h>
#include <bgame/allocator/frame.h>
#include <bgame/scene.h>
#include <bgame/ui.h>
#include <bgame/ui/animation.h>
#include <bgame/asset.h>
#include <bgame/asset/9patch.h>
#include <cute_app.h>
#include <cute_draw.h>
#include <cute_sprite.h>
#include <cute_input.h>
#include <cute_math.h>
#include <bhash.h>

BGAME_DECLARE_TRACKED_ALLOCATOR(main_scene_alloc)

typedef BHASH_TABLE(const char*, CF_Sprite) sprite_table_t;

BGAME_VAR(CF_Sprite, test_sprite) = { 0 };
BGAME_VAR(sprite_table_t, sprite_instances) = { 0 };

BGAME_VAR(bgame_asset_bundle_t*, main_scene_assets) = NULL;
BGAME_VAR(bool, sort_animations) = false;
static bgame_9patch_t* window_border = NULL;

static void
animate_bbox(Clay_RenderCommand* from, const Clay_RenderCommand* to) {
	float factor = 0.2f;
	from->boundingBox.x = cf_lerp(from->boundingBox.x, to->boundingBox.x, factor);
	from->boundingBox.y = cf_lerp(from->boundingBox.y, to->boundingBox.y, factor);
	from->boundingBox.width = cf_lerp(from->boundingBox.width, to->boundingBox.width, factor);
	from->boundingBox.height = cf_lerp(from->boundingBox.height, to->boundingBox.height, factor);
}

bgame_ui_animator_t bbox_animator = {
	.transition = animate_bbox,
};

static int
compare_anim_names(const void* lhs, const void* rhs) {
	const CF_Sprite* lhs_sprite = *(const CF_Sprite**)lhs;
	const CF_Sprite* rhs_sprite = *(const CF_Sprite**)rhs;
	return strcmp(lhs_sprite->animation->name, rhs_sprite->animation->name);
}

static void
init(int argc, const char** argv) {
	if (!test_sprite.name) {
		test_sprite = cf_make_demo_sprite();
	}

	bhash_config_t config = bhash_config_default();
	config.memctx = main_scene_alloc;
	bhash_reinit(&sprite_instances, config);

	Clay_SetDebugModeEnabled(true);

	bgame_asset_begin_load(&main_scene_assets);
	window_border = bgame_load_9patch(
		main_scene_assets,
		"/assets/frame.png",
		(bgame_9patch_config_t) {
			.left = 25,
			.right = 25,
			.top = 25,
			.bottom = 25,
		}
	);
	bgame_asset_end_load(main_scene_assets);
}

static void
cleanup(void) {
	bgame_asset_destroy_bundle(main_scene_assets);
	bhash_cleanup(&sprite_instances);
}

static void
fixed_update(void* udata) {
	(void)udata;
}

static void
update(void) {
	bgame_asset_check_bundle(main_scene_assets);

	cf_app_update(fixed_update);
	cf_clear_color(0.5f, 0.5f, 0.5f, 1.f);

	if (cf_key_just_pressed(CF_KEY_SPACE)) {
		sort_animations = !sort_animations;
	}

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
				.nine_patch = window_border,
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
				.nine_patch = window_border,
			})
		) {
			CLAY_TEXT(
				sort_animations ? CLAY_STRING("Animations by name") : CLAY_STRING("Animations by index"),
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
					.childGap = 16,
				})
			) {
				// Create a separate instance for each animation
				// This has to be done in a separate pass since the sprite address
				// can change.
				for (int i = 0; i < hsize(test_sprite.animations); ++i) {
					const CF_Animation* animation = test_sprite.animations[i];
					const char* anim_name = animation->name;

					if (!bhash_is_valid(bhash_find(&sprite_instances, anim_name))) {
						bhash_put(&sprite_instances, anim_name, test_sprite);
						bhash_index_t index = bhash_find(&sprite_instances, anim_name);
						cf_sprite_play(&sprite_instances.values[index], anim_name);
					}
				}

				// Load animation to a temporary array for sorting
				bhash_index_t num_animations = bhash_len(&sprite_instances);
				CF_Sprite** animations = bgame_alloc_for_frame(
					sizeof(CF_Sprite*) * num_animations,
					_Alignof(CF_Sprite*)
				);
				for (int i = 0; i < num_animations; ++i) {
					CF_Sprite* instance = &sprite_instances.values[i];
					cf_sprite_update(instance);
					animations[i] = instance;
				}
				if (sort_animations) {
					qsort(animations, num_animations, sizeof(CF_Sprite*), compare_anim_names);
				}

				for (int i = 0; i < num_animations; ++i) {
					CF_Sprite* instance = animations[i];
					Clay_String anim_name = {
						.length = strlen(instance->animation->name),
						.chars = instance->animation->name,
					};

					CLAY(
						Clay__AttachId(
							Clay__HashString(anim_name, 0, Clay__GetParentElementId())
						),
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
						}),
						CLAY_TRANSFORM({ .animator = &bbox_animator })
					) {
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
							anim_name,
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
