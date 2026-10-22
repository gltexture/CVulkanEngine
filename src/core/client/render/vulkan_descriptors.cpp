//
// Created by ded on 12.10.2026.
//

#include "vulkan_descriptors.h"

#include "vulkan_buffers.h"
#include "vulkan_config.hpp"
#include "vulkan_render_core.h"

#include "../../utility/logger.h"

namespace cvulkan::client::render::vk {
    CVulkanDescriptorsPool::CVulkanDescriptorsPool(const CVulkanContext& context, const std::vector<CVulkanDescriptorTypeCount>& descriptorTypeCounts) : _context{context}, _descriptorTypeCounts{descriptorTypeCounts} {
        logging::debug("Creating vkDescriptorPool");

        std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.reserve(_descriptorTypeCounts.size());
        uint32_t maxSets = 0;
        for (uint32_t i = 0; i < _descriptorTypeCounts.size(); ++i) {
            const auto& [_descType, _descCount] = _descriptorTypeCounts.at(i);
            maxSets += _descCount;
            poolSizes.push_back({
                .type = _descType,
                .descriptorCount = _descCount
            });
        }

        const VkDescriptorPoolCreateInfo poolCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = maxSets,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };
        vk::util::vkCheck(vkCreateDescriptorPool(context.device().vkDevice(), &poolCreateInfo, nullptr, &this->_vkDescriptorPool), "Failed to create descriptor pool");
    }

    CVulkanDescriptorsPool::~CVulkanDescriptorsPool() {
        if (this->_vkDescriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(this->_context.device().vkDevice(), this->_vkDescriptorPool, nullptr);
            this->_vkDescriptorPool = VK_NULL_HANDLE;
            logging::debug("Destroyed vkDescriptorPool");
        }
    }

    void CVulkanDescriptorsPool::freeDescriptorSet(const VkDescriptorSet& descriptorSet) const {
        vk::util::vkCheck(vkFreeDescriptorSets(this->_context.device().vkDevice(), this->_vkDescriptorPool, 1, &descriptorSet), "Failed to free descriptor set");
    }

    CVulkanDescriptorSetLayout::CVulkanDescriptorSetLayout(const CVulkanContext& context, const std::vector<CVulkanLayoutInfo>&& layoutInfoArray) : _context{context}, _layoutInfoArray{std::move(layoutInfoArray)} {
        logging::debug("Creating vkDescriptorSetLayout");

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(layoutInfoArray.size());

        for (const auto& [_descType, _binding, _descCount, _stage] : layoutInfoArray) {
            layoutBindings.push_back({
                .binding = _binding,
                .descriptorType = _descType,
                .descriptorCount = _descCount,
                .stageFlags = _stage
            });
        }

        const VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data()
        };
        vk::util::vkCheck(vkCreateDescriptorSetLayout(context.device().vkDevice(), &vkDescriptorSetLayoutCreateInfo, nullptr, &this->_vkDescriptorSetLayout), "Failed to create descriptor set layout");
    }

    CVulkanDescriptorSetLayout::~CVulkanDescriptorSetLayout() {
        if (this->_vkDescriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(this->_context.device().vkDevice(), this->_vkDescriptorSetLayout, nullptr);
            this->_vkDescriptorSetLayout = VK_NULL_HANDLE;
            logging::debug("Destroyed vkDescriptorSetLayout");
        }
    }

    CVulkanDescriptorSet::CVulkanDescriptorSet(const CVulkanContext& context, const CVulkanDescriptorsPool& descriptorsPool, const CVulkanDescriptorSetLayout& vkDescriptorSetLayout) : _context{context}, _descriptorsPool{descriptorsPool} {
        const VkDescriptorSetLayout layout = vkDescriptorSetLayout.vkDescriptorSetLayout();

        const VkDescriptorSetAllocateInfo descriptorAllocateInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorsPool.vkDescriptorPool(),
            .descriptorSetCount = 1,
            .pSetLayouts = &layout
        };

        vk::util::vkCheck(vkAllocateDescriptorSets(context.device().vkDevice(), &descriptorAllocateInfo, &this->_vkDescriptorSet), "Failed to create descriptor set");
    }

    CVulkanDescriptorSet::~CVulkanDescriptorSet() {

    }

    void CVulkanDescriptorSet::setBuffer(const CVulkanBuffer& buffer, const VkDeviceSize range, const uint32_t binding, const VkDescriptorType descriptorType) const {
        const VkDescriptorBufferInfo bufferInfo {
            .buffer = buffer.vkBuffer(),
            .offset = 0,
            .range = range,
        };

        const VkWriteDescriptorSet writeDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = this->_vkDescriptorSet,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = descriptorType,
            .pBufferInfo = &bufferInfo
        };

        vkUpdateDescriptorSets(this->_context.device().vkDevice(), 1, &writeDescriptorSet, 0, nullptr);
    }

    void CVulkanDescriptorSet::setImages(const std::vector<const CVulkanImage2DView*>& imageViews, const CVulkanSampler& sampler, const uint32_t baseBinding) const {
        const uint32_t numImages = imageViews.size();
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(numImages);

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(imageViews.size());

        for (uint32_t i = 0; i < imageViews.size(); ++i) {
            const auto& imageView = imageViews[i];

            imageInfos.push_back({
                .sampler = sampler.vkSampler(),
                .imageView = imageView->vkImageView(),
                .imageLayout = imageView->depthImage() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });

            writes.push_back({
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = this->_vkDescriptorSet,
                .dstBinding = baseBinding + i,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = nullptr
            });

            writes.back().pImageInfo = &imageInfos.back();
        }


        vkUpdateDescriptorSets(this->_context.device().vkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    void CVulkanDescriptorSet::setImagesArray(const std::vector<const CVulkanImage2DView*>& imageViews, const CVulkanSampler& sampler, const uint32_t baseBinding) const {
        const uint32_t numImages = static_cast<uint32_t>(imageViews.size());

        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(numImages);

        for (const auto& imageView : imageViews) {
            imageInfos.push_back({
                .sampler = sampler.vkSampler(),
                .imageView = imageView->vkImageView(),
                .imageLayout = imageView->depthImage() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
        }

        const VkWriteDescriptorSet write {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = _vkDescriptorSet,
            .dstBinding = baseBinding,
            .dstArrayElement = 0,
            .descriptorCount = numImages,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = imageInfos.data()
        };

        vkUpdateDescriptorSets(this->_context.device().vkDevice(), 1, &write, 0, nullptr);
    }

    CVulkanDescriptorAllocator::CVulkanDescriptorAllocator(const CVulkanContext& context) : _context(context) {
        this->createDescriptorLimits();
        logging::info("Created vkDescriptor allocator");
    }

    CVulkanDescriptorAllocator::~CVulkanDescriptorAllocator() {
    }

    std::vector<CVulkanDescriptorSet>& CVulkanDescriptorAllocator::allocateDescriptorSets(const std::string& id, const uint32_t count, const CVulkanDescriptorSetLayout& layout) {
        CVulkanDescriptorPoolInfo* targetPool {};
        uint32_t poolPos = 0;

        for (uint32_t i = 0; i < this->_descriptorPoolInfoList.size(); ++i) {
            CVulkanDescriptorPoolInfo& descriptorPoolInfo = this->_descriptorPoolInfoList[i];
            bool hasCapacity = true;
            for (const auto& layoutInfo : layout.layoutInfoArray()) {
                const VkDescriptorType descriptorType = layoutInfo._descType;
                if (!descriptorPoolInfo._descriptorCount.contains(descriptorType)) {
                    throw std::runtime_error(std::format("Descriptor set type {} not found", static_cast<uint32_t>(descriptorType)));
                }
                const uint32_t maxTotal = this->_descriptorLimits.at(descriptorType);
                if (count > maxTotal) {
                    throw std::runtime_error(std::format("Cannot create more than {} for descriptor type {}", maxTotal, static_cast<uint32_t>(descriptorType)));
                }
                if (descriptorPoolInfo._descriptorCount.at(descriptorType) < count) {
                    hasCapacity = false;
                    break;
                }
            }
            if (hasCapacity) {
                targetPool = &descriptorPoolInfo;
                poolPos = i;
                break;
            }
        }

        if (targetPool == nullptr) {
            CVulkanDescriptorPoolInfo descriptorPoolInfo = this->createDescriptorPoolInfo();
            this->_descriptorPoolInfoList.push_back(std::move(descriptorPoolInfo));
            targetPool = &this->_descriptorPoolInfoList.back();
            poolPos = static_cast<uint32_t>(this->_descriptorPoolInfoList.size() - 1);
        }

        if (this->_descriptorSetInfoMap.contains(id)) {
            throw std::runtime_error(std::format("Descriptor set ID '{}' already exists", id));
        }

        std::vector<CVulkanDescriptorSet> result {};
        result.reserve(count);
        for (uint32_t i = 0; i < count; i++) {
            CVulkanDescriptorSet descriptorSet {this->_context, targetPool->_descriptorsPool, layout};
            result.push_back(std::move(descriptorSet));
        }
        this->_descriptorSetInfoMap.emplace(id, CVulkanDescriptorSetInfo{std::move(result), poolPos});

        for (const auto& t : layout.layoutInfoArray()) {
            const VkDescriptorType& descType = t._descType;
            targetPool->_descriptorCount.at(descType) -= count;
        }

        return this->_descriptorSetInfoMap.at(id)._descriptorSets;
    }

    void CVulkanDescriptorAllocator::createDescriptorLimits() {
        const VkPhysicalDeviceLimits& limits =_context.physicalDevice().vkPhysicalDeviceProperties().limits;
        this->_descriptorLimits.emplace(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, std::min(renderConfig::MAX_DESCRIPTORS, limits.maxDescriptorSetUniformBuffers));
        this->_descriptorLimits.emplace(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, std::min(renderConfig::MAX_DESCRIPTORS, limits.maxDescriptorSetSamplers));
        this->_descriptorLimits.emplace(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, std::min(renderConfig::MAX_DESCRIPTORS, limits.maxDescriptorSetStorageBuffers));
    }

    CVulkanDescriptorPoolInfo CVulkanDescriptorAllocator::createDescriptorPoolInfo() {
        std::vector<CVulkanDescriptorTypeCount> descTypeCounts;
        descTypeCounts.reserve(this->_descriptorLimits.size());
        for (const auto& [type, count] : this->_descriptorLimits) {
            descTypeCounts.push_back({type, count});
        }
        return {this->_descriptorLimits, {this->_context, descTypeCounts}};
    }

    void CVulkanDescriptorAllocator::freeDescriptorSet(const std::string& id) {
        if (!this->_descriptorSetInfoMap.contains(id)) {
            throw std::runtime_error(std::format("Descriptor set ID '{}' not found", id));
        }
        const CVulkanDescriptorSetInfo& descriptorSetInfo = this->_descriptorSetInfoMap.at(id);
        if (descriptorSetInfo._poolPos >= this->_descriptorPoolInfoList.size()) {
            throw std::runtime_error(std::format("Could not find descriptor pool associated to set with id {}", id));
        }
        CVulkanDescriptorPoolInfo& descriptorPoolInfo = this->_descriptorPoolInfoList.at(descriptorSetInfo._poolPos);
        for (const auto& t : descriptorSetInfo._descriptorSets) {
            descriptorPoolInfo._descriptorsPool.freeDescriptorSet(t.vkDescriptorSet());
        }
    }

    const CVulkanDescriptorSet& CVulkanDescriptorAllocator::getDescriptorSet(const std::string& id, uint32_t pos) const {
        const auto it = this->_descriptorSetInfoMap.find(id);
        if (it == this->_descriptorSetInfoMap.end()) {
            throw std::runtime_error(std::format("Descriptor set ID '{}' not found", id));
        }
        auto& descriptorSets = it->second._descriptorSets;
        if (pos >= descriptorSets.size()) {
            throw std::runtime_error(std::format("Descriptor set position {} is out of range", pos));
        }
        return descriptorSets.at(pos);
    }
}
