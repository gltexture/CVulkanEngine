//
// Created by ded on 15.09.2026.
//

#pragma once

#include <string_view>
#include "vulkan_utility.h"

namespace cvulkan::client::render::core {
    class CVulkanContext;
}

namespace cvulkan::client::render::shader {
    class CVulkanShaderModule {
    public:
        CVulkanShaderModule(const core::CVulkanContext& context, std::string_view shaderSpvFileName, VkShaderStageFlagBits stage);
        ~CVulkanShaderModule();

        CVULKAN_NO_COPY(CVulkanShaderModule);

        CVulkanShaderModule(CVulkanShaderModule&&) noexcept = default;

        CVulkanShaderModule& operator=(CVulkanShaderModule&&) = delete;

        [[nodiscard]] VkShaderModule vkShaderModule() const {
            return _vkShaderModule;
        }

        [[nodiscard]] const VkShaderStageFlagBits& stage() const {
            return _stage;
        }

    private:
        const core::CVulkanContext& _context;
        const VkShaderStageFlagBits _stage;
        VkShaderModule _vkShaderModule{};
    };

    std::vector<char> compileShader(const std::string& shaderName, const std::string& shaderCode, uint32_t shaderType);
    void compileShaderIfOutOfDate(const std::string& shaderName, uint32_t shaderType);
}
