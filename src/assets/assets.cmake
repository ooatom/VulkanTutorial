function(copyAsset INPUT_FILE OUTPUT_FILE)
    add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${CMAKE_COMMAND} -E copy ${INPUT_FILE} ${OUTPUT_FILE}
            DEPENDS ${INPUT_FILE}
            COMMENT "Asset copied."
    )
endfunction(copyAsset)

set(INPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/assets)
set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assets)

file(MAKE_DIRECTORY ${OUTPUT_DIRECTORY})
file(GLOB_RECURSE INPUT_FILES ${INPUT_DIRECTORY}/*)
list(REMOVE_ITEM INPUT_FILES ${INPUT_DIRECTORY}/assets.cmake)
set(OUTPUT_FILES "")

foreach (INPUT_FILE ${INPUT_FILES})
    file(RELATIVE_PATH relativePath ${INPUT_DIRECTORY} ${INPUT_FILE})
    set(OUTPUT_FILE ${OUTPUT_DIRECTORY}/${relativePath})

    list(APPEND OUTPUT_FILES ${OUTPUT_FILE})
    copyAsset(${INPUT_FILE} ${OUTPUT_FILE})
endforeach ()

add_custom_target(assetCopy DEPENDS ${OUTPUT_FILES})