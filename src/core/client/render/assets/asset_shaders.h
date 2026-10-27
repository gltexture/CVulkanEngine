//
// Created by ded on 30.09.2026.
//

#pragma once
#include <filesystem>

#include "client/render/vulkan_shaders.h"

namespace cvulkan::io_utils {
    class CVulkanResourcePath;
}

namespace cvulkan::client::render::asset {
    [[nodiscard]] inline shaderc_shader_kind toShadercKind(const VkShaderStageFlagBits stage) {
        switch (stage) {
            case VK_SHADER_STAGE_VERTEX_BIT:
                return shaderc_glsl_vertex_shader;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                return shaderc_glsl_fragment_shader;
            case VK_SHADER_STAGE_COMPUTE_BIT:
                return shaderc_glsl_compute_shader;
            case VK_SHADER_STAGE_GEOMETRY_BIT:
                return shaderc_glsl_geometry_shader;
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                return shaderc_glsl_tess_control_shader;
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                return shaderc_glsl_tess_evaluation_shader;
            default:
                throw std::invalid_argument("Unsupported Vulkan shader stage");
        }
    }

    namespace shaderCompile {
        std::vector<char> compileShader(const std::string& shaderName, const std::string& shaderCode, uint32_t shaderType);
        void compileShaderIfOutOfDate(const io_utils::CVulkanResourcePath& shaderSourceFile, uint32_t shaderType);
    }

    namespace shaderLoad {
        shader::CVulkanShaderModule compileAndLoadShader(const vk::CVulkanContext& context, const io_utils::CVulkanResourcePath& shaderSourceFile, VkShaderStageFlagBits vkStage);
    }
}
