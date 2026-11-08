//
// Created by ded on 15.09.2026.
//

#pragma once

#include <string_view>
#include "utility/sys_utils.h"

namespace cvulkan::io_utils {
    class CVulkanResourcePath;
}

namespace cvulkan::client::render::vk {
    class CVulkanContext;
}

namespace cvulkan::client::render::shader {
    class CVulkanShaderModule {
    public:
        CVulkanShaderModule(const vk::CVulkanContext& context, const io_utils::CVulkanResourcePath& shaderSpvFileName, VkShaderStageFlagBits vkStage, const VkSpecializationInfo* vkSpecializationInfo);
        ~CVulkanShaderModule();

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanShaderModule);

        CVulkanShaderModule(CVulkanShaderModule&&) noexcept = default;

        [[nodiscard]] VkShaderModule vkShaderModule() const {
            return _vkShaderModule;
        }

        [[nodiscard]] const VkShaderStageFlagBits& vkStage() const {
            return _vkStage;
        }

        [[nodiscard]] const VkSpecializationInfo* vkSpecializationInfo() const {
            return _vkSpecializationInfo;
        }

    private:
        const vk::CVulkanContext& _context;
        const VkShaderStageFlagBits _vkStage;
        const VkSpecializationInfo* _vkSpecializationInfo{};
        VkShaderModule _vkShaderModule{};
    };
}
