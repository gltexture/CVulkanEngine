cmake_minimum_required(VERSION 3.20)
project(CVulkanEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)

set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        SPIRV-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG vulkan-sdk-1.4.363.0
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        SPIRV-Tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG v2026.4
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG vulkan-sdk-1.4.363.0
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG v2026.4
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.5
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG v3.5.0
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.93.0-docking
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        CrashCatch
        GIT_REPOSITORY https://github.com/keithpotz/CrashCatch
        GIT_TAG v1.6.0
        GIT_SHALLOW TRUE
)

FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY https://github.com/jrouwe/JoltPhysics
        GIT_TAG v5.7.0
        GIT_SHALLOW TRUE
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
        vma
        imgui
        CrashCatch
        JoltPhysics
)

add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
)

find_package(Vulkan REQUIRED)

function(configure_engine_target target)
    target_compile_features(${target} PUBLIC cxx_std_23)

    target_compile_definitions(${target} PUBLIC
            GLM_FORCE_DEPTH_ZERO_TO_ONE

            $<$<CONFIG:Debug>:CVULKAN_DEV_MODE>
            $<$<CONFIG:Debug>:CVULKAN_DEBUG_MODE>

            $<$<CONFIG:RelWithDebInfo>:CVULKAN_DEV_MODE>
            $<$<CONFIG:RelWithDebInfo>:CVULKAN_DEBUG_MODE>
    )

    target_compile_options(${target} PRIVATE
            $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU,Clang>>:-O3>
            $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU,Clang>>:-g>
            $<$<AND:$<CONFIG:RelWithDebInfo>,$<CXX_COMPILER_ID:GNU,Clang>>:-g>
            $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang>>:-g>
    )
endfunction()

# Core

add_library(engine_core STATIC
        src/core/utility/system.cpp
        src/core/utility/io.cpp
        src/core/utility/functionality.cpp
        src/core/utility/exceptions.cpp
        src/core/physics/jolt_core.cpp
        src/core/physics/jolt_core.h
)

FetchContent_GetProperties(
        Jolt
        SOURCE_DIR _jolt_source_dir
)

target_include_directories(engine_core PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/libs
        ${CMAKE_BINARY_DIR}/_deps/joltphysics-src
)

target_link_libraries(engine_core PUBLIC
        glm::glm
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        CrashCatch
        Jolt
)

target_link_libraries(engine_core PRIVATE
        stdc++exp
)

configure_engine_target(engine_core)

target_precompile_headers(engine_core PRIVATE
        <glm/glm.hpp>
        <glm/gtc/matrix_transform.hpp>
        <glm/gtc/quaternion.hpp>
        <spdlog/spdlog.h>
        <vector>
        <array>
        <memory>
        <string>
        <string_view>
        <optional>
        <variant>
        <functional>
        <algorithm>
        <utility>
        <type_traits>
        <concepts>
        <cstdint>
        <cstddef>
        <cstring>
        <filesystem>
        <unordered_map>
        <unordered_set>
        <map>
        <set>
        <chrono>
        <thread>
        <mutex>
        <atomic>
        <format>
        <nlohmann/json.hpp>
)

# Client

add_library(engine_client STATIC
        src/client/client_runtime.cpp

        src/client/render/glfw_window.cpp
        src/client/control/client_input.cpp
        src/client/control/client_input__handling.cpp

        src/client/render/vulkan_render_core.cpp
        src/client/render/vulkan_ext.cpp
        src/client/render/vulkan_commands.cpp
        src/client/render/vulkan_synchronization.cpp
        src/client/render/vulkan_swapchain.cpp
        src/client/render/vulkan_buffers.cpp
        src/client/render/vulkan_shaders.cpp
        src/client/render/vulkan_pipeline.cpp
        src/client/render/vulkan_images.cpp
        src/client/render/vulkan_transform.cpp
        src/client/render/vulkan_descriptors.cpp
        src/client/render/vulkan_mem_alloc.cpp

        src/client/render/_impl/stbi_impl.cpp
        src/client/render/_impl/stbiw_impl.cpp
        src/client/render/_impl/vma_impl.cpp

        src/client/render/scene/vulkan_render_loop.cpp
        src/client/render/scene/vulkan_render_structs.cpp
        src/client/render/scene/vulkan_render_cache.cpp
        src/client/render/scene/vulkan_scene_objects.cpp
        src/client/render/scene/vulkan_render_scene__deferred_context.cpp
        src/client/render/scene/vulkan_render_scene__post.cpp
        src/client/render/scene/vulkan_render_scene__swapchain.cpp
        src/client/render/scene/vulkan_render_scene__imgui.cpp
        src/client/render/scene/cvulkan_render_mrt.cpp
        src/client/render/scene/vulkan_render_scene__deferred_lpass.cpp
        src/client/render/scene/vulkan_render_scene__deferred_gpass.cpp

        src/client/render/scene/camera/view_camera.cpp

        src/client/render/assets/asset_shaders.cpp
        src/client/render/assets/asset_models.cpp
        src/client/render/assets/asset_textures.cpp

        src/client/render/scene/debug/vulkan_imgui_window_renderer.cpp
        src/client/test_logic/test_logic.cpp
)

target_include_directories(engine_client PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/libs
)

target_link_libraries(engine_client
        PUBLIC
        engine_core
        Vulkan::Vulkan
        glfw
        glm::glm

        PRIVATE
        spdlog::spdlog
        shaderc_combined
        assimp
        nlohmann_json::nlohmann_json
        GPUOpen::VulkanMemoryAllocator
        imgui
        CrashCatch
)

configure_engine_target(engine_client)

target_precompile_headers(engine_client PRIVATE
        <vulkan/vulkan.hpp>
        <vulkan/vulkan_core.h>
        <GLFW/glfw3.h>
        <glm/glm.hpp>
        <glm/gtc/matrix_transform.hpp>
        <glm/gtc/type_ptr.hpp>
        <glm/gtc/quaternion.hpp>
        <spdlog/spdlog.h>
        <shaderc/shaderc.h>
        <vk_mem_alloc.h>
        <vector>
        <array>
        <memory>
        <string>
        <string_view>
        <optional>
        <variant>
        <functional>
        <algorithm>
        <unordered_map>
        <unordered_set>
        <cstdint>
        <cstring>
)

# Server

add_library(engine_server STATIC
        src/server/server_runtime.cpp
        src/server/server_runtime.h
)

target_link_libraries(engine_server INTERFACE
        engine_core
)

target_include_directories(engine_server PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/libs
)

# Client

add_executable(client_app
        src/main.cpp
)

target_link_libraries(client_app PRIVATE
        engine_client
)