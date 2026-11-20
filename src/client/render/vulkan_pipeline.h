//
// Created by ded on 16.09.2026.
//

#pragma once

#include "core/utility/system.h"
#include <span>

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
        CVulkanPipelineBuildInfo(
            std::span<const CRef_CVulkanDescriptorSetLayout> descriptorSetLayouts,
            std::span<const CRef_CVulkanShaderModule> shaderModules,
            std::span<const CRef_CVulkanPushConstantRange> pushConstantRanges,
            const VkPipelineVertexInputStateCreateInfo& vkVertexInputInfo,
            std::span<const VkFormat> vkColorFormats,
            VkFormat vkDepthFormat,
            bool useBlend
        );

        CVulkanPipelineBuildInfo(
            std::span<const CRef_CVulkanDescriptorSetLayout> descriptorSetLayouts,
            std::span<const CRef_CVulkanShaderModule> shaderModules,
            std::span<const CRef_CVulkanPushConstantRange> pushConstantRanges,
            const VkPipelineVertexInputStateCreateInfo& vkVertexInputInfo,
            std::span<const VkFormat> vkColorFormats,
            bool useBlend
        );

        CVulkanPipelineBuildInfo(
            std::span<const CRef_CVulkanDescriptorSetLayout> descriptorSetLayouts,
            std::span<const CRef_CVulkanShaderModule> shaderModules,
            std::span<const CRef_CVulkanPushConstantRange> pushConstantRanges,
            const VkPipelineVertexInputStateCreateInfo& vkVertexInputInfo,
            std::initializer_list<VkFormat> vkColorFormats,
            VkFormat vkDepthFormat,
            bool useBlend
        );

        CVulkanPipelineBuildInfo(
            std::span<const CRef_CVulkanDescriptorSetLayout> descriptorSetLayouts,
            std::span<const CRef_CVulkanShaderModule> shaderModules,
            std::span<const CRef_CVulkanPushConstantRange> pushConstantRanges,
            const VkPipelineVertexInputStateCreateInfo& vkVertexInputInfo,
            std::initializer_list<VkFormat> vkColorFormats,
            bool useBlend
        );

        ~CVulkanPipelineBuildInfo() = default;

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanPipelineBuildInfo);

        [[nodiscard]] std::span<const VkFormat> vkColorFormats() const {
            return _vkColorFormats;
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
        const std::span<const VkFormat> _vkColorFormats{};
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

    protected:
        static void validateVertexInputState(const VkPipelineVertexInputStateCreateInfo& info);

    private:
        const CVulkanContext& _context;
        VkPipeline _vkPipeline{};
        VkPipelineLayout _vkPipelineLayout{};
    };
}
