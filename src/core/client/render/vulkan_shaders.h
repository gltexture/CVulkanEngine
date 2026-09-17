//
// Created by ded on 15.09.2026.
//

#pragma once

#include <string_view>
#include <vulkan/vulkan_core.h>
#include "vulkan_utility.h"
#include "shaderc/shaderc.h"

namespace cvulkan::client::render::core {
    class CVulkanContext;
}

namespace cvulkan::client::render::shader {
    class CVulkanShaderModule {
    public:
        explicit CVulkanShaderModule(const core::CVulkanContext& context, const VkShaderStageFlagBits stage)
            : _context(context), _stage(stage) {}
        ~CVulkanShaderModule() = default;

        CVULKAN_NO_COPY(CVulkanShaderModule);

        CVulkanShaderModule(CVulkanShaderModule&&) noexcept = default;
        CVulkanShaderModule& operator=(CVulkanShaderModule&&) = delete;

        void createShaderModule(std::string_view shaderSpvFileName);
        void destroyShaderModule();

        [[nodiscard]] VkShaderModule vkShaderModule() const {
            return _vkShaderModule;
        }

        [[nodiscard]] VkShaderStageFlagBits stage() const {
            return _stage;
        }

    private:
        const core::CVulkanContext& _context;
        const VkShaderStageFlagBits _stage;
        VkShaderModule _vkShaderModule {};
    };

    std::vector<char> compileShader(const std::string& shaderName, std::string& shaderCode, shaderc_shader_kind shaderType);
    void compileShaderIfOutOfDate(const std::string& shaderName, shaderc_shader_kind shaderType);

    inline shaderc_shader_kind toShadercShaderKind(const VkShaderStageFlagBits stage) {
        switch (stage) {
            case VK_SHADER_STAGE_VERTEX_BIT:
                return shaderc_glsl_vertex_shader;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                return shaderc_glsl_fragment_shader;
            case VK_SHADER_STAGE_GEOMETRY_BIT:
                return shaderc_glsl_geometry_shader;
            case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
                return shaderc_glsl_tess_control_shader;
            case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
                return shaderc_glsl_tess_evaluation_shader;
            default:
                throw std::runtime_error("Unknown shader stage");
        }
    }
}
