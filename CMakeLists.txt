cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 23)
project(renderer)

# optimization level
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
add_executable(${PROJECT_NAME} src/main.cpp src/Application.cpp src/MeshRenderer.cpp src/ParticleRenderer.cpp)

include(src/shaders/shaders.cmake)
add_dependencies(${PROJECT_NAME} shaderCompilation)

include(src/assets/assets.cmake)
add_dependencies(${PROJECT_NAME} assetCopy)

# -------- Vulkan --------
set(VULKAN_ROOT $ENV{HOME}/VulkanSDK/1.3.250.1/macOS)
target_include_directories(${PROJECT_NAME} PUBLIC "${VULKAN_ROOT}/include")
target_link_libraries(${PROJECT_NAME} PUBLIC "${VULKAN_ROOT}/lib/libvulkan.1.3.250.dylib")

#add_library(Vulkan SHARED IMPORTED)
#set_target_properties(Vulkan PROPERTIES
#        INCLUDE_DIRECTORIES "${VULKAN_ROOT}/include"
#        IMPORTED_LOCATION "${VULKAN_ROOT}/lib/libvulkan.1.3.250.dylib")
#target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan)

target_compile_definitions(${PROJECT_NAME} PRIVATE
VK_ICD_FILENAMES=${VULKAN_ROOT}/share/vulkan/icd.d/MoltenVK_icd.json
VK_LAYER_PATH=${VULKAN_ROOT}/share/vulkan/explicit_layer.d)

# -------- glm --------
#set(CMAKE_FIND_DEBUG_MODE TRUE)
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
#set(CMAKE_FIND_DEBUG_MODE FALSE)

# -------- glfw3 --------
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

#set(HEADER_GLFW "/usr/local/include/GLFW")
#set(LIB_GLFW "/usr/local/lib/libglfw.3.3.dylib")
#target_include_directories(${PROJECT_NAME} PUBLIC ${HEADER_GLFW})
#target_link_libraries(${PROJECT_NAME} PUBLIC ${LIB_GLFW})

# -------- imgui --------
set(IMGUI_ROOT "./libs/imgui")
file(GLOB_RECURSE IMGUI_SOURCES "${IMGUI_ROOT}/*.cpp")
add_library(ImGUI STATIC ${IMGUI_SOURCES})
target_include_directories(ImGUI PUBLIC ${IMGUI_ROOT} "${IMGUI_ROOT}/backends")
target_link_libraries(ImGUI PUBLIC glfw)

target_link_libraries(${PROJECT_NAME} PUBLIC ImGUI)

# -------- libs --------
target_include_directories(${PROJECT_NAME} PUBLIC "./libs")
