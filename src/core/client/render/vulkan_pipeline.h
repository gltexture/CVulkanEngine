//
// Created by ded on 16.09.2026.
//

#pragma once
#include "utility/sys_utils.h"

namespace cvulkan::client::render::shader {
    class CVulkanShaderModule;
}

namespace cvulkan::client::render::vk {
    struct CVulkanPushConstantRange;
    class CVulkanDescriptorSetLayout;

    using Ref_CVulkanDescriptorSetLayout = std::reference_wrapper<const CVulkanDescriptorSetLayout>;
    using Ref_CVulkanShaderModule = std::reference_wrapper<const shader::CVulkanShaderModule>;
    using Ref_CVulkanPushConstantRange = std::reference_wrapper<const CVulkanPushConstantRange>;

    class CVulkanContext;

    struct CVulkanPushConstantRange {
        uint32_t stage;
        uint32_t offset;
        uint32_t size;
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
        CVulkanPipelineBuildInfo(const std::vector<Ref_CVulkanDescriptorSetLayout>& descriptorSetLayouts, const std::vector<Ref_CVulkanShaderModule>& shaderModules, const std::vector<Ref_CVulkanPushConstantRange>& pushConstantRanges, const VkPipelineVertexInputStateCreateInfo& vertexInputInfo, const VkFormat& colorFormat, const VkFormat& depthFormat);
        CVulkanPipelineBuildInfo(const std::vector<Ref_CVulkanDescriptorSetLayout>& descriptorSetLayouts, const std::vector<Ref_CVulkanShaderModule>& shaderModules, const std::vector<Ref_CVulkanPushConstantRange>& pushConstantRanges, const VkPipelineVertexInputStateCreateInfo& vertexInputInfo, const VkFormat& colorFormat)
            : CVulkanPipelineBuildInfo((descriptorSetLayouts), (shaderModules), (pushConstantRanges), vertexInputInfo, colorFormat, VK_FORMAT_UNDEFINED) {};
        ~CVulkanPipelineBuildInfo() = default;

        CLASS_NO_COPY(CVulkanPipelineBuildInfo);

        [[nodiscard]] VkFormat colorFormat() const {
            return _colorFormat;
        }

        [[nodiscard]] VkFormat depthFormat() const {
            return _depthFormat;
        }

        [[nodiscard]] const std::vector<Ref_CVulkanPushConstantRange>& pushConstantRanges() const {
            return _pushConstantRanges;
        }

        [[nodiscard]] const std::vector<Ref_CVulkanShaderModule>& shaderModules() const {
            return _shaderModules;
        }

        [[nodiscard]] const std::vector<Ref_CVulkanDescriptorSetLayout>& descriptorSetLayouts() const {
            return _descriptorSetLayouts;
        }

        [[nodiscard]] const VkPipelineVertexInputStateCreateInfo& vertexInputInfo() const {
            return _vertexInputInfo;
        }

    private:
        VkFormat _colorFormat{};
        VkFormat _depthFormat{};
        const std::vector<Ref_CVulkanPushConstantRange>& _pushConstantRanges;
        const std::vector<Ref_CVulkanShaderModule>& _shaderModules;
        const std::vector<Ref_CVulkanDescriptorSetLayout>& _descriptorSetLayouts;
        VkPipelineVertexInputStateCreateInfo _vertexInputInfo{};
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
