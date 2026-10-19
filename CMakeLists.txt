cmake_minimum_required(VERSION 3.20) 
project(CVulkanEngine)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_BUILD_PARALLEL_LEVEL 8)

include(FetchContent)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
)

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        SPIRV-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG main
)
FetchContent_Declare(
        SPIRV-Tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG main
)
FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG main
)
FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG v2026.4
)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.5
)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(
        spdlog
        SPIRV-Headers
        SPIRV-Tools
        glslang
        shaderc
        glfw
        glm
        assimp
        json
)

find_package(Vulkan REQUIRED)
add_executable(CVulkanEngine
        src/main.cpp
        src/core/client/client_runtime.cpp
        src/core/client/render/glfw_window.cpp
        src/core/client/control/client_mouse_control.cpp
        src/core/client/control/client_keyboard_control.cpp
        src/core/client/control/client_controls.cpp
        src/core/client/render/vulkan_render_core.cpp
        src/core/client/render/vulkan_utility.cpp
        src/core/client/render/vulkan_ext.cpp
        src/core/client/render/vulkan_commands.cpp
        src/core/client/render/vulkan_synchronization.cpp
        src/core/client/render/vulkan_swapchain.cpp
        src/core/client/render/vulkan_render_loop.cpp
        src/core/client/render/vulkan_buffers.cpp
        src/core/client/render/scene/vulkan_render_scene.cpp
        src/core/client/render/scene/vulkan_render_structs.cpp
        src/core/client/render/scene/vulkan_render_cache.cpp
        src/core/client/render/vulkan_shaders.cpp
        src/core/client/render/vulkan_pipeline.cpp
        src/core/engine_context.cpp
        src/core/client/render/vulkan_images.cpp
        src/core/client/render/vulkan_images.h
        src/core/client/render/vulkan_transform.cpp
        src/core/client/render/vulkan_transform.h
        src/core/client/render/scene/vulkan_scene_objects.cpp
        src/core/client/render/scene/vulkan_scene_objects.h
        src/core/client/render/assets/asset_shaders.cpp
        src/core/client/render/assets/asset_shaders.h
        src/core/client/render/assets/asset_models.cpp
        src/core/client/render/assets/asset_models.h
        src/core/client/render/assets/asset_textures.cpp
        src/core/client/render/assets/asset_textures.h
        src/core/util/collections.hpp
        src/core/client/render/vulkan_descriptors.cpp
        src/core/client/render/vulkan_descriptors.h
        src/core/client/render/vulkan_descriptors.cpp
        src/core/client/render/_impl/stbi_impl.cpp
        src/core/client/render/_impl/stbi_impl.cpp
)

add_link_options("-fuse-ld=lld")

target_compile_definitions(CVulkanEngine PRIVATE
        GLM_FORCE_DEPTH_ZERO_TO_ONE
)

target_precompile_headers(CVulkanEngine PRIVATE
        <vulkan/vulkan_core.h>
        <GLFW/glfw3.h>
        <glm/glm.hpp>
        <spdlog/spdlog.h>
        <vector>
        <memory>
        <string>
        <string_view>
        <shaderc/shaderc.h>
)

target_link_libraries(
        CVulkanEngine
        PRIVATE Vulkan::Vulkan
        PRIVATE glfw
        PRIVATE glm::glm
        PRIVATE spdlog::spdlog
        PRIVATE shaderc_combined
        PRIVATE assimp
        PRIVATE nlohmann_json::nlohmann_json
)

target_include_directories(CVulkanEngine
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external
)
target_include_directories(CVulkanEngine
        PRIVATE ${CMAKE_SOURCE_DIR}/src/core
)