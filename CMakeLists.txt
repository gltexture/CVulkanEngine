cmake_minimum_required(VERSION 3.20) 
project(CVulkanEngine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
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
        GIT_TAG v2026.4
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

FetchContent_Declare(
        vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG v3.5.0
)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.93.0-docking
)

FetchContent_Declare(
        CrashCatch
        GIT_REPOSITORY https://github.com/keithpotz/CrashCatch
        GIT_TAG v1.6.0
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
add_executable(CVulkanEngine
        src/main.cpp
        src/core/client/client_runtime.cpp
        src/core/client/render/glfw_window.cpp
        src/core/client/control/client_input.cpp
        src/core/client/render/vulkan_render_core.cpp
        src/core/client/render/vulkan_ext.cpp
        src/core/client/render/vulkan_commands.cpp
        src/core/client/render/vulkan_synchronization.cpp
        src/core/client/render/vulkan_swapchain.cpp
        src/core/client/render/scene/vulkan_render_loop.cpp
        src/core/client/render/vulkan_buffers.cpp
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
        src/core/utility/collections.hpp
        src/core/client/render/vulkan_descriptors.cpp
        src/core/client/render/vulkan_descriptors.h
        src/core/client/render/_impl/stbi_impl.cpp
        src/core/client/render/_impl/stbiw_impl.cpp
        src/core/utility/system.cpp
        src/core/utility/system.h
        src/core/utility/io.cpp
        src/core/utility/io.h
        src/core/client/render/scene/vulkan_render_scene__deferred_context.cpp
        src/core/client/render/scene/camera/view_camera.cpp
        src/core/client/render/scene/camera/view_camera.h
        src/core/test_logic/test_logic.cpp
        src/core/test_logic/test_logic.h
        src/core/client/render/_impl/vma_impl.cpp
        src/core/client/render/vulkan_mem_alloc.cpp
        src/core/client/render/vulkan_mem_alloc.h
        src/core/client/render/scene/vulkan_render_scene__post.cpp
        src/core/client/render/scene/vulkan_render_scene__post.h
        src/core/client/render/vulkan_spec_const.tpp
        src/core/client/render/vulkan_spec_const.h
        src/core/utility/functionality.cpp
        src/core/utility/functionality.h
        src/core/client/render/scene/vulkan_render_scene__swapchain.cpp
        src/core/client/render/scene/vulkan_render_scene__swapchain.h
        src/core/client/render/scene/vulkan_render_scene__imgui.cpp
        src/core/client/render/scene/vulkan_render_scene__imgui.h
        src/core/utility/exceptions.cpp
        src/core/utility/exceptions.h
        src/core/client/control/client_input__handling.cpp
        src/core/utility/DEBUG.tpp
        src/core/utility/DEBUG.h
        src/core/client/render/scene/cvulkan_render_mrt.cpp
        src/core/client/render/scene/cvulkan_render_mrt.h
        src/core/client/render/scene/vulkan_render_scene__deferred_lpass.cpp
        src/core/client/render/scene/vulkan_render_scene__deferred_gpass.cpp
        src/core/client/render/scene/debug/vulkan_imgui_window_renderer.cpp
        src/core/client/render/scene/debug/vulkan_imgui_window_renderer.h
)

target_compile_features(CVulkanEngine PRIVATE cxx_std_23)

target_compile_options(CVulkanEngine PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-g>
        $<$<CONFIG:RelWithDebInfo>:-g>
        $<$<CONFIG:Debug>:-g>
)

target_link_options(CVulkanEngine PRIVATE
        $<$<CONFIG:Debug>:-Wl,--pdb=>
        $<$<CONFIG:RelWithDebInfo>:-Wl,--pdb=>
        $<$<CONFIG:Release>:-Wl,--pdb=>
)

target_compile_definitions(CVulkanEngine PRIVATE
        GLM_FORCE_DEPTH_ZERO_TO_ONE

        $<$<CONFIG:Debug>:CVULKAN_DEV_MODE>
        $<$<CONFIG:Debug>:CVULKAN_DEBUG_MODE>

        $<$<CONFIG:RelWithDebInfo>:CVULKAN_DEV_MODE>
        $<$<CONFIG:RelWithDebInfo>:CVULKAN_DEBUG_MODE>
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

target_link_libraries(CVulkanEngine PRIVATE stdc++exp)

target_link_libraries(
        CVulkanEngine
        PRIVATE
        Vulkan::Vulkan
        glfw
        glm::glm
        spdlog::spdlog
        shaderc_combined
        assimp
        nlohmann_json::nlohmann_json
        GPUOpen::VulkanMemoryAllocator
        imgui
        CrashCatch
)

target_include_directories(CVulkanEngine
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs
)
target_include_directories(CVulkanEngine
        PRIVATE ${CMAKE_SOURCE_DIR}/src/core
)