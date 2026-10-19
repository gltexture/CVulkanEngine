//
// Created by ded on 16.09.2026.
//

#include "vulkan_pipeline.h"

#include <ranges>

#include "vulkan_render_core.h"
#include "vulkan_shaders.h"
#include "vulkan_utility.h"

namespace cvulkan::client::render::vk {
    CVulkanPipelineCache::CVulkanPipelineCache(const CVulkanContext& context) : _context{context} {
        constexpr VkPipelineCacheCreateInfo pipelineCacheCreateInfo  {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        };
        utility::vkCheck(vkCreatePipelineCache(this->_context.device().vkDevice(), &pipelineCacheCreateInfo, nullptr, &this->_vkPipelineCache), "Error creating vkPipeline cache");
        logging::info("Created vkPipelineCache");
    }

    CVulkanPipelineCache::~CVulkanPipelineCache() {
        if (this->_vkPipelineCache != VK_NULL_HANDLE) {
            vkDestroyPipelineCache(this->_context.device().vkDevice(), this->_vkPipelineCache, nullptr);
            logging::info("Destroyed vkPipelineCache");
            this->_vkPipelineCache = VK_NULL_HANDLE;
        }
    }

    CVulkanPipelineBuildInfo::CVulkanPipelineBuildInfo(const std::vector<Ref_CVulkanDescriptorSetLayout>& descriptorSetLayouts, const std::vector<Ref_CVulkanShaderModule>& shaderModules, const std::vector<Ref_CVulkanPushConstantRange>& pushConstantRanges, const VkPipelineVertexInputStateCreateInfo& vertexInputInfo, const VkFormat& colorFormat, const VkFormat& depthFormat)
        : _colorFormat(colorFormat), _depthFormat(depthFormat), _pushConstantRanges(pushConstantRanges), _shaderModules(shaderModules), _descriptorSetLayouts(descriptorSetLayouts), _vertexInputInfo(vertexInputInfo) {
    }

    CVulkanPipeline::CVulkanPipeline(const CVulkanContext& context, const CVulkanPipelineBuildInfo& buildInfo) : _context{context} {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        shaderStages.reserve(buildInfo.shaderModules().size());
        for (const auto& t: buildInfo.shaderModules()) {
            const VkPipelineShaderStageCreateInfo shaderStageCreateInfo  {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = t.get().stage(),
                .module = t.get().vkShaderModule(),
                .pName = "main"
            };
            shaderStages.emplace_back(shaderStageCreateInfo);
        }
        const VkPipelineInputAssemblyStateCreateInfo inputAssemblyState  {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        };
        const VkPipelineViewportStateCreateInfo viewportState  {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1
        };
        const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE, //VK_CULL_MODE_BACK_BIT
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .lineWidth = 1.0F
        };
        const VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
        };
        constexpr std::array dynamicStates{
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        const VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };
        const VkPipelineColorBlendAttachmentState blendAttachmentState{
            .blendEnable = VK_FALSE,
            .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
        };
        const VkPipelineColorBlendStateCreateInfo colorBlendState{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &blendAttachmentState
        };
        const VkFormat colorFormat = buildInfo.colorFormat();
        const VkFormat depthFormat = buildInfo.depthFormat();
        std::optional<VkPipelineDepthStencilStateCreateInfo> depthStencilStateCreateInfoOptional{};
        if (buildInfo.depthFormat() != VK_FORMAT_UNDEFINED) {
            constexpr VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo  {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                .depthTestEnable = VK_TRUE,
                .depthWriteEnable = VK_TRUE,
                .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                .depthBoundsTestEnable = VK_FALSE,
                .stencilTestEnable = VK_FALSE,
            };
            depthStencilStateCreateInfoOptional.emplace(depthStencilStateCreateInfo);
        }
        VkPipelineRenderingCreateInfo renderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorFormat,
            .depthAttachmentFormat = depthFormat
        };

        std::vector<VkPushConstantRange> pushConstantRanges {};
        pushConstantRanges.reserve(buildInfo.pushConstantRanges().size());
        for (const auto& t : buildInfo.pushConstantRanges()) {
            const VkPushConstantRange pushConstantRange {
                .stageFlags = t.get().stage,
                .offset = t.get().offset,
                .size = t.get().size
            };
            pushConstantRanges.push_back(pushConstantRange);
        }

        const auto layouts = buildInfo.descriptorSetLayouts() | std::views::transform([](const auto& t) {
            return t.get().vkDescriptorSetLayout();
        });
        const std::vector descriptorSetLayouts (layouts.begin(), layouts.end());

        const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
            .pSetLayouts = descriptorSetLayouts.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
            .pPushConstantRanges = pushConstantRanges.data()
        };


        utility::vkCheck(vkCreatePipelineLayout(this->_context.device().vkDevice(), &pipelineLayoutCreateInfo, nullptr, &this->_vkPipelineLayout), "Failed to create pipeline layout");
        VkGraphicsPipelineCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &renderingCreateInfo,
            .stageCount = static_cast<uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &buildInfo.vertexInputInfo(),
            .pInputAssemblyState = &inputAssemblyState,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizationStateCreateInfo,
            .pMultisampleState = &multisampleStateCreateInfo,
            .pColorBlendState = &colorBlendState,
            .pDynamicState = &dynamicStateCreateInfo,
            .layout = this->_vkPipelineLayout,
            .renderPass = VK_NULL_HANDLE
        };
        if (depthStencilStateCreateInfoOptional.has_value()) {
            createInfo.pDepthStencilState = &depthStencilStateCreateInfoOptional.value();
        }
        utility::vkCheck(vkCreateGraphicsPipelines(this->_context.device().vkDevice(), this->_context.pipelineCache().vkPipelineCache(), 1, &createInfo, nullptr, &this->_vkPipeline), "Error creating graphics pipeline");
    }

    CVulkanPipeline::~CVulkanPipeline() {
        if (this->_vkPipelineLayout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(this->_context.device().vkDevice(), this->_vkPipelineLayout, nullptr);
        }
        if (this->_vkPipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(this->_context.device().vkDevice(), this->_vkPipeline, nullptr);
        }
    }
}
