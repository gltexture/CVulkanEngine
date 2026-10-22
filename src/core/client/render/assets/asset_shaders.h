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
    namespace shaderCompile {
        std::vector<char> compileShader(const std::string& shaderName, const std::string& shaderCode, uint32_t shaderType);
        void compileShaderIfOutOfDate(const io_utils::CVulkanResourcePath& shaderSourceFile, uint32_t shaderType);
    }

    namespace shaderLoad {
        shader::CVulkanShaderModule compileAndLoadShader(const vk::CVulkanContext& context, const io_utils::CVulkanResourcePath& shaderSourceFile, uint32_t shaderType, VkShaderStageFlagBits vkStage);
    }
}
