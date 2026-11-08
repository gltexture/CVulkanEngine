//
// Created by ded on 16.09.2026.
//

#pragma once
#include "utility/sys_utils.h"

namespace cvulkan::client::render::shader {
    class CVulkanShaderModule;
}

namespace cvulkan::client::render::vk {
    class CVulkanDescriptorSetLayout;
    struct CVulkanPushConstantRange;
    class CVulkanContext;

    using CRef_CVulkanDescriptorSetLayout = const std::reference_wrapper<const CVulkanDescriptorSetLayout>;
    using CRef_CVulkanShaderModule = const std::reference_wrapper<const shader::CVulkanShaderModule>;
    using CRef_CVulkanPushConstantRange = const std::reference_wrapper<const CVulkanPushConstantRange>;

    struct CVulkanPushConstantRange {
        uint32_t _stage;
        uint32_t _offset;
        uint32_t _size;
    };

    class CVulkanPipelineCache {
    public:
        explicit CVulkanPipelineCache(const CVulkanContext& context);

        ~CVulkanPipelineCache();

        CLASS_NO_COPY(CVulkanPipelineCache);

        [[nodiscard]] VkPipelineCache vkPipelineCache() const {
            return _vkPipelineCache;
        }

    private:
        const CVulkanContext& _context;
        VkPipelineCache _vkPipelineCache{};
    };

    class CVulkanPipelineBuildInfo {
    public:
        CVulkanPipelineBuildInfo(std::span<CRef_CVulkanDescriptorSetLayout> descriptorSetLayouts,
                                 std::span<CRef_CVulkanShaderModule> shaderModules,
                                 std::span<CRef_CVulkanPushConstantRange> pushConstantRanges,
                                 const VkPipelineVertexInputStateCreateInfo& vkVertexInputInfo,
                                 VkFormat vkColorFormat, VkFormat vkDepthFormat, bool useBlend);

        CVulkanPipelineBuildInfo(const std::span<CRef_CVulkanDescriptorSetLayout> descriptorSetLayouts,
                                 const std::span<CRef_CVulkanShaderModule> shaderModules,
                                 const std::span<CRef_CVulkanPushConstantRange> pushConstantRanges,
                                 const VkPipelineVertexInputStateCreateInfo& vkVertexInputInfo,
                                 const VkFormat vkColorFormat, const bool useBlend)
            : CVulkanPipelineBuildInfo((descriptorSetLayouts), (shaderModules), (pushConstantRanges), vkVertexInputInfo, vkColorFormat, VK_FORMAT_UNDEFINED, useBlend) {
        };

        ~CVulkanPipelineBuildInfo() = default;

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanPipelineBuildInfo);

        [[nodiscard]] VkFormat vkColorFormat() const {
            return _vkColorFormat;
        }

        [[nodiscard]] VkFormat vkDepthFormat() const {
            return _vkDepthFormat;
        }

        [[nodiscard]] std::span<CRef_CVulkanPushConstantRange> pushConstantRanges() const {
            return _pushConstantRanges;
        }

        [[nodiscard]] std::span<CRef_CVulkanShaderModule> shaderModules() const {
            return _shaderModules;
        }

        [[nodiscard]] std::span<CRef_CVulkanDescriptorSetLayout> descriptorSetLayouts() const {
            return _descriptorSetLayouts;
        }

        [[nodiscard]] const VkPipelineVertexInputStateCreateInfo& vkVertexInputInfo() const {
            return _vkVertexInputInfo;
        }

        [[nodiscard]] bool isUseBlend() const {
            return _useBlend;
        }

    private:
        const VkFormat _vkColorFormat{};
        const VkFormat _vkDepthFormat{};
        const std::span<CRef_CVulkanPushConstantRange> _pushConstantRanges;
        const std::span<CRef_CVulkanShaderModule> _shaderModules;
        const std::span<CRef_CVulkanDescriptorSetLayout> _descriptorSetLayouts;
        const VkPipelineVertexInputStateCreateInfo _vkVertexInputInfo{};
        const bool _useBlend;
    };

    class CVulkanPipeline {
    public:
        CVulkanPipeline(const CVulkanContext& context, const CVulkanPipelineBuildInfo& buildInfo);

        ~CVulkanPipeline();

        CLASS_NO_COPY(CVulkanPipeline);

        [[nodiscard]] VkPipeline vkPipeline() const {
            return _vkPipeline;
        }

        [[nodiscard]] VkPipelineLayout vkPipelineLayout() const {
            return _vkPipelineLayout;
        }

    private:
        const CVulkanContext& _context;
        VkPipeline _vkPipeline{};
        VkPipelineLayout _vkPipelineLayout{};
    };
}
