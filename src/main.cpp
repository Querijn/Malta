#include "sokol.hpp"

#include "shaders/quad.hpp"

#include "stb_image.h"

#if __EMSCRIPTEN__
	#include <emscripten.h>
#endif

using i8  = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = long long;

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;

static struct
{
	sg_pass_action passAction;
	sg_pipeline	   pip;
	sg_bindings	   bind;

	float dayCycleTime  = 0.0f;
	float waterTime	    = 0.0f;
	float windFrequency = 5.0f;
} state;

sg_image MakeImage(const char* path)
{
	int		 w, h, comp;
	u8*		 image = stbi_load(path, &w, &h, &comp, 4);
	sg_image img   = sg_make_image({ .width		   = w,
									 .height	   = h,
									 .pixel_format = sg_pixel_format::SG_PIXELFORMAT_RGBA8,
									 .data		   = { .subimage = { { { .ptr = image, .size = (size_t)(w * h * 4) } } } },
									 .label		   = path });
	stbi_image_free(image);
	return img;
}

void init(void)
{
	sg_setup(
		{
			.logger		 = { .func = slog_func },
			.environment = sglue_environment(),
		});

	// clang-format off
	const float vertices[] =
	{
		-1.0f, -1.0f,
		 3.0f, -1.0f,
		-1.0f,  3.0f
	};
	// clang-format on

	state.bind.images[IMG_dayBackTex]  = MakeImage("assets/day-hero-background.png");
	state.bind.images[IMG_dayFrontTex] = MakeImage("assets/day-hero-foreground.png");

	state.bind.images[IMG_eveBackTex]  = MakeImage("assets/evening-hero-background.png");
	state.bind.images[IMG_eveFrontTex] = MakeImage("assets/evening-hero-foreground.png");

	state.bind.images[IMG_nightBackTex]	 = MakeImage("assets/night-hero-background.png");
	state.bind.images[IMG_nightFrontTex] = MakeImage("assets/night-hero-foreground.png");

	state.bind.images[IMG_waterMask] = MakeImage("assets/water-mask.png");
	state.bind.images[IMG_windMap]	 = MakeImage("assets/wind-map.png");
	state.bind.images[IMG_starMap]	 = MakeImage("assets/star-map.png");

	state.bind.vertex_buffers[0] = sg_make_buffer({ .data  = SG_RANGE(vertices),
													.label = "quad-vertices" });

	u16 indices[]			= { 0, 1, 2 };
	state.bind.index_buffer = sg_make_buffer({ .type  = SG_BUFFERTYPE_INDEXBUFFER,
											   .data  = SG_RANGE(indices),
											   .label = "quad-indices" });

	state.bind.samplers[0] = sg_make_sampler({ .min_filter = SG_FILTER_NEAREST,
											   .mag_filter = SG_FILTER_NEAREST,
											   .wrap_u	   = SG_WRAP_MIRRORED_REPEAT,
											   .wrap_v	   = SG_WRAP_MIRRORED_REPEAT,
											   .label	   = "quad-sampler" });

	state.pip = sg_make_pipeline({ .shader = sg_make_shader(quad_shader_desc(sg_query_backend())),
								   .layout = {
									   .attrs = {
										   { .format = SG_VERTEXFORMAT_FLOAT2 },
									   } },
								   .index_type = SG_INDEXTYPE_UINT16,
								   .label	   = "quad-pipeline" });

	// clang-format off
	state.passAction = 
	{
		.colors =  { { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f } } }
	};
	// clang-format on
}

f32 floor(f32 x)
{
	u64 n = (u64)x;
	f32 d = (f32)n;
	if (d == x || x >= 0)
		return d;
	return d - 1;
}

float fract01(float x)
{
	return x - floor(x);
}

void frame(void)
{
	sg_begin_pass({ .action = state.passAction, .swapchain = sglue_swapchain() });
	sg_apply_pipeline(state.pip);

#if _WIN32
	char title[64];
	snprintf(title, 64, "Malta %1.2f", state.dayCycleTime);
	sapp_set_window_title(title);
#endif

	VertParams_t vertParams;
	vertParams.screenSize[0] = sapp_widthf();
	vertParams.screenSize[1] = sapp_heightf();

	vertParams._imageSize[0] = 480.0f;
	vertParams._imageSize[1] = 300.0f;

#if __EMSCRIPTEN__
	float dayDuration = (float)EM_ASM_DOUBLE({ return Module.dayDuration * 60.0; });
#else
	float dayDuration = 100.0f;
#endif

	state.dayCycleTime = fract01(state.dayCycleTime + (float)sapp_frame_duration() / dayDuration);
	state.waterTime	   = fract01(state.waterTime + (float)sapp_frame_duration() / 10.0f);

	FragParams_t fragParams = {};
	fragParams.dayCycleTime = state.dayCycleTime;
	fragParams.waterTime	= state.waterTime;

#if __EMSCRIPTEN__
	fragParams.windFrequency = (float)EM_ASM_DOUBLE({ return Module.windSpeed; });
	fragParams.edge			 = (float)EM_ASM_DOUBLE({ return Module.edgePerc / 100.0; });
#else
	fragParams.windFrequency = 5.0f;
	fragParams.edge			 = 0.1f;
#endif

	sg_apply_uniforms(0, SG_RANGE(vertParams));
	sg_apply_uniforms(1, SG_RANGE(fragParams));
	sg_apply_bindings(&state.bind);
	sg_draw(0, 3, 1);
	sg_end_pass();
	sg_commit();
}

void event(const sapp_event* e)
{
	(void)e;

	switch(e->type)
	{
	case SAPP_EVENTTYPE_KEY_DOWN:
#if !__EMSCRIPTEN__
		if (e->key_code == SAPP_KEYCODE_ESCAPE)
			sapp_quit();
#endif
		break;

	default: // -Wswitch
		break;
	}
}

void cleanup(void)
{
	sg_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	return {
		.init_cb	  = init,
		.frame_cb	  = frame,
		.cleanup_cb	  = cleanup,
		.event_cb	  = event,
		.width		  = 3 * 480,
		.height		  = 3 * 300,
		.window_title = "Malta",
		.icon		  = { .sokol_default = false },
		.logger		  = { .func = slog_func },
	};
}
