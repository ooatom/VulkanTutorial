set(GLSLC /Users/oo/VulkanSDK/1.3.250.1/macOS/bin/glslc)

function(compileShader INPUT_FILE OUTPUT_FILE)
    add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${GLSLC} ${INPUT_FILE} -o ${OUTPUT_FILE}
            DEPENDS ${INPUT_FILE}
            COMMENT "Shader compiled."
    )
endfunction(compileShader)

set(INPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/shaders)
set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${OUTPUT_DIRECTORY})
file(GLOB_RECURSE INPUT_FILES ${INPUT_DIRECTORY}/*.vert ${INPUT_DIRECTORY}/*.frag ${INPUT_DIRECTORY}/*.comp)
set(OUTPUT_FILES "")

foreach (INPUT_FILE ${INPUT_FILES})
#    get_filename_component(fileName ${INPUT_FILE} NAME)
    file(RELATIVE_PATH relativePath ${INPUT_DIRECTORY} ${INPUT_FILE})
    set(OUTPUT_FILE ${OUTPUT_DIRECTORY}/${relativePath}.spv)

    list(APPEND OUTPUT_FILES ${OUTPUT_FILE})
    compileShader(${INPUT_FILE} ${OUTPUT_FILE})
endforeach ()

add_custom_target(shaderCompilation DEPENDS ${OUTPUT_FILES})