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

FetchContent_MakeAvailable(spdlog)

find_package(Vulkan REQUIRED)
add_executable(CVulkanEngine
        src/main.cpp
        src/core/util/logger.inl
        src/core/client/client_runtime.cpp
        src/core/client/render/client_window.cpp
        src/core/engine_context.cpp
        src/core/engine_context.h
        src/core/client/render/client_window.h
        src/core/client/client_runtime.h
        src/core/client/render/client_window.cpp
        src/core/client/control/client_mouse_control.cpp
        src/core/client/control/client_mouse_control.h
        src/core/client/control/client_keyboard_control.cpp
        src/core/client/control/client_keyboard_control.h
        src/core/client/control/client_controls.cpp
        src/core/client/control/client_controls.h
        src/core/client/render/client_render_core.cpp
        src/core/client/render/client_render_core.h
        src/core/client/render/client_render_config.h
        src/core/client/render/vulkanPrograms/vulkan_utility.cpp
        src/core/client/render/vulkanPrograms/vulkan_utility.h
        src/core/client/render/vulkanPrograms/vulkan_ext.cpp
        src/core/client/render/vulkanPrograms/vulkan_ext.h
)

target_link_libraries(
    CVulkanEngine
    PRIVATE Vulkan::Vulkan
    PRIVATE glfw
    PRIVATE glm::glm
    PRIVATE spdlog::spdlog
)

target_include_directories(CVulkanEngine PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
)