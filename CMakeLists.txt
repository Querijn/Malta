# Default 
cmake_minimum_required(VERSION 3.16)
project(Malta)
include(FetchContent)

add_subdirectory(ShaderCompiler)

# Sokol
FetchContent_Declare(
	sokol
	GIT_REPOSITORY	https://github.com/Querijn/sokol.git
	GIT_TAG			ec404440ba3b5aefe49f1fd18d12650fe09a8e3e
)
FetchContent_MakeAvailable(sokol)

# STB
FetchContent_Declare(
	stb
	GIT_REPOSITORY	https://github.com/nothings/stb.git
	GIT_TAG			ae721c50eaf761660b4f90cc590453cdb0c2acd0
)
FetchContent_MakeAvailable(stb)

# Shader sources
file(GLOB MALTA_SHADER_SRC		"src/shaders/*.glsl")
source_group("Shaders"			FILES	${MALTA_SHADER_SRC})
list(APPEND MALTA_SRC			${MALTA_SHADER_SRC})

add_executable(Malta
	${MALTA_SHADER_SRC}
    "src/sokol.cpp" "src/sokol.hpp"
    "src/stb.cpp"
    "src/main.cpp"
)

target_include_directories(Malta PRIVATE src)
target_include_directories(Malta PUBLIC  ${sokol_SOURCE_DIR})
target_include_directories(Malta PRIVATE ${stb_SOURCE_DIR})
set_target_properties(Malta PROPERTIES
						VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
						CXX_STANDARD 20
						CXX_EXTENSIONS ON)
add_shader_generator(Malta ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders)

if(MSVC)
	target_compile_options(Malta PRIVATE /W4 /WX)
else()
	target_compile_options(Malta PRIVATE -Wall -Wextra -Werror -Wno-missing-field-initializers)
endif()

if (WIN32)
	target_compile_definitions(Malta PUBLIC WIN32_LEAN_AND_MEAN)
	target_compile_definitions(Malta PUBLIC NOMINMAX)
	target_compile_definitions(Malta PUBLIC SOKOL_WIN32_FORCE_MAIN)
	target_compile_definitions(Malta PUBLIC SOKOL_WIN32_FORCE_WINMAIN)
elseif (EMSCRIPTEN)
	# Some emscripten specific constants
	# All default flags
	list(APPEND MALTA_FLAGS 
		"-sASSERTIONS"
		"-lwebgl.js"
		"-sMALLOC=emmalloc"
		"-sUSE_WEBGL2=1"
		# "-sFETCH=1"
		"-Os"
		"--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets"
		# "--shell-file ${CMAKE_CURRENT_LIST_DIR}/shell.html"
	)

	# Flags for the current target
	if (NOT MALTA_SUBMISSION)
		list(APPEND MALTA_FLAGS
			"-gsource-map"
			"-sWARN_UNALIGNED=1"
			"-fsanitize=undefined"
			"-fsanitize=null"
			# "-fsanitize=address"
		)
	else ()
		list(APPEND MALTA_FLAGS
			"-g0"
			"-Os"
			# "-sDISABLE_EXCEPTION_CATCHING=2"
			"-sMINIMAL_RUNTIME" # This doesn't work with fetch
			# "-sSAFE_HEAP=1"
			"-sUSE_CLOSURE_COMPILER=1"
		)
	endif ()

	# Turn to a string and set the flags
	list(JOIN MALTA_FLAGS " " MALTA_FLAGS)
	message("${CMAKE_BUILD_TYPE} Emscripten flags: ${MALTA_FLAGS}")
    set_target_properties(Malta PROPERTIES LINK_FLAGS ${MALTA_FLAGS})
endif ()
