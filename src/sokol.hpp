#pragma once

#if _WIN32
	// #define SOKOL_GLCORE 1
	#define SOKOL_D3D11 1
#elif defined(__EMSCRIPTEN__)
	#define SOKOL_GLES3 1
#elif defined(__APPLE__)
	// #define SOKOL_GLCORE 1
	#define SOKOL_METAL 1
#endif

// Ignore GCC import-preprocessor warnings
#if defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wimport-preprocessor-directive-pedantic"
#endif

#include <sokol_app.h>
#include <sokol_log.h>
#include <sokol_gfx.h>
#include <sokol_audio.h>
#include <util/sokol_gl.h>
#include <util/sokol_shape.h>
#include <sokol_glue.h>

#if SOKOL_IMGUI
#include <imgui.h>
#include <util/sokol_imgui.h>
#endif

#if defined(__GNUC__)
	#pragma GCC diagnostic pop
#endif