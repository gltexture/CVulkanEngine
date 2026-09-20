//
// Created by ded on 16.09.2026.
//

#pragma once

#include "vulkan_shaders.h"

namespace cvulkan::client::render::core {
    class CVulkanContext;

    class CVulkanPipelineCache {
    public:
        explicit CVulkanPipelineCache(const CVulkanContext& context)
            : _context(context) {
        }
        ~CVulkanPipelineCache();

        CVULKAN_NO_COPY(CVulkanPipelineCache);

        void createPipelineCache();

        [[nodiscard]] VkPipelineCache vkPipelineCache() const {
            return _vkPipelineCache;
        }

    private:
        const CVulkanContext& _context;
        VkPipelineCache _vkPipelineCache{};
    };

    class CVulkanPipelineBuildInfo {
    public:
        CVulkanPipelineBuildInfo(const std::vector<shader::CVulkanShaderModule>& shaderModules, const VkPipelineVertexInputStateCreateInfo& vertexInputInfo, const VkFormat colorFormat)
            : _colorFormat(colorFormat), _shaderModules(shaderModules), _vertexInputInfo(vertexInputInfo) {
        }

        CVULKAN_NO_COPY(CVulkanPipelineBuildInfo);

        [[nodiscard]] VkFormat colorFormat() const {
            return _colorFormat;
        }

        [[nodiscard]] const std::vector<shader::CVulkanShaderModule>& shaderModules() const {
            return _shaderModules;
        }

        [[nodiscard]] const VkPipelineVertexInputStateCreateInfo& vertexInputInfo() const {
            return _vertexInputInfo;
        }

    private:
        VkFormat _colorFormat{};
        const std::vector<shader::CVulkanShaderModule>& _shaderModules;
        VkPipelineVertexInputStateCreateInfo _vertexInputInfo{};
    };

    class CVulkanPipeline {
    public:
        explicit CVulkanPipeline(const CVulkanContext& context)
            : _context(context) {
        }

        ~CVulkanPipeline();

        CVULKAN_NO_COPY(CVulkanPipeline);

        void createPipeline(const CVulkanPipelineBuildInfo& buildInfo);

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
