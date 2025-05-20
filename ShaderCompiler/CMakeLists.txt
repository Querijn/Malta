FetchContent_Declare(
	sokol_tools
	GIT_REPOSITORY https://github.com/floooh/sokol-tools-bin
	GIT_TAG        8582d2a57ebd10ef629d269e98ef7e97ab0ecf98
)
FetchContent_MakeAvailable(sokol_tools)

set(ShaderGenerator_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")
set(ShaderGenerator_TOOL_DIR ${sokol_tools_SOURCE_DIR} CACHE INTERNAL "")

function(ADD_SHADER_GENERATOR project outputFolder)
	add_custom_target(${project}_ShaderGenerator ${SHADER_TEMPLATES} COMMAND cmd /c "node ${ShaderGenerator_SOURCE_DIR}/build_shader.js \"${ShaderGenerator_TOOL_DIR}\" \"${outputFolder}\"" WORKING_DIRECTORY ${outputFolder})
	set_target_properties(${project}_ShaderGenerator PROPERTIES FOLDER "")
endfunction(ADD_SHADER_GENERATOR)

