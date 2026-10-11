//
// Created by ded on 15.09.2026.
//

#pragma once

#include <string_view>
#include <vector>

#include "vulkan_utility.h"

namespace cvulkan::client::render::vk {
    class CVulkanContext;
}

namespace cvulkan::client::render::shader {
    class CVulkanShaderModule {
    public:
        CVulkanShaderModule(const vk::CVulkanContext& context, std::string_view shaderSpvFileName, VkShaderStageFlagBits stage);
        ~CVulkanShaderModule();

        CVULKAN_NO_COPY_NO_ASSIGN_MOVE(CVulkanShaderModule);

        CVulkanShaderModule(CVulkanShaderModule&&) noexcept = default;

        [[nodiscard]] VkShaderModule vkShaderModule() const {
            return _vkShaderModule;
        }

        [[nodiscard]] const VkShaderStageFlagBits& stage() const {
            return _stage;
        }

    private:
        const vk::CVulkanContext& _context;
        const VkShaderStageFlagBits _stage;
        VkShaderModule _vkShaderModule{};
    };
}
