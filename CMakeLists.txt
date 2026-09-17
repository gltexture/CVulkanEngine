cmake_minimum_required(VERSION 3.20) 
project(CVulkanEngine)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

include(FetchContent)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)

set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        SPIRV-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG  main
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

FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(
        SPIRV-Headers
        SPIRV-Tools
        glslang
        shaderc
)

find_package(Vulkan REQUIRED)
add_executable(CVulkanEngine
        src/main.cpp
        src/core/util/logger.inl
        src/core/client/client_runtime.cpp
        src/core/client/render/glfw_window.cpp
        src/core/engine_context.cpp
        src/core/engine_context.h
        src/core/client/render/glfw_window.h
        src/core/client/client_runtime.h
        src/core/client/render/glfw_window.cpp
        src/core/client/control/client_mouse_control.cpp
        src/core/client/control/client_mouse_control.h
        src/core/client/control/client_keyboard_control.cpp
        src/core/client/control/client_keyboard_control.h
        src/core/client/control/client_controls.cpp
        src/core/client/control/client_controls.h
        src/core/client/render/vulkan_render_core.cpp
        src/core/client/render/vulkan_render_core.h
        src/core/client/render/vulkan_utility.cpp
        src/core/client/render/vulkan_utility.h
        src/core/client/render/vulkan_ext.cpp
        src/core/client/render/vulkan_ext.h
        src/core/client/render/vulkan_commands.cpp
        src/core/client/render/vulkan_commands.h
        src/core/client/render/vulkan_synchronization.cpp
        src/core/client/render/vulkan_synchronization.h
        src/core/client/render/vulkan_swapchain.cpp
        src/core/client/render/vulkan_swapchain.h
        src/core/client/render/vulkan_render_loop.cpp
        src/core/client/render/vulkan_render_loop.h
        src/core/client/render/vulkan_config.h
        src/core/client/render/scene/vulkan_render_scene.cpp
        src/core/client/render/scene/vulkan_render_scene.h
        src/core/client/render/vulkan_buffers.cpp
        src/core/client/render/vulkan_buffers.h
        src/core/client/render/scene/vulkan_render_structs.cpp
        src/core/client/render/scene/vulkan_render_structs.h
        src/core/client/render/scene/vulkan_render_cache.cpp
        src/core/client/render/scene/vulkan_render_cache.h
        src/core/client/render/vulkan_shaders.cpp
        src/core/client/render/vulkan_shaders.h
        src/core/client/render/vulkan_pipeline.cpp
        src/core/client/render/vulkan_pipeline.h
)

target_link_libraries(
    CVulkanEngine
    PRIVATE Vulkan::Vulkan
    PRIVATE glfw
    PRIVATE glm::glm
    PRIVATE spdlog::spdlog
    PRIVATE shaderc_combined
)

target_include_directories(CVulkanEngine
    PRIVATE ${CMAKE_SOURCE_DIR}/src/core
)

add_custom_command(TARGET CVulkanEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/resources
        $<TARGET_FILE_DIR:CVulkanEngine>/resources
)