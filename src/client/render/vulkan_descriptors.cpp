//
// Created by ded on 12.10.2026.
//

#include "vulkan_descriptors.h"

#include "vulkan_buffers.h"
#include "vulkan_config.hpp"
#include "vulkan_render_core.h"
#include "core/utility/debug.h"
#include "core/utility/exceptions.h"

#include "core/utility/logging.h"

namespace cvulkan::client::render::vk {
    CVulkanDescriptorsPool::CVulkanDescriptorsPool(const CVulkanContext& context, const std::vector<CVulkanDescriptorTypeCount>& descriptorTypeCounts) : _context{context}, _descriptorTypeCounts{descriptorTypeCounts} {
        log::debug("Creating Vulkan descriptor pool");

        std::vector<VkDescriptorPoolSize> poolSizes;
        poolSizes.reserve(_descriptorTypeCounts.size());
        uint32_t maxSets = 0;
        for (auto [_descType, _descCount]: _descriptorTypeCounts) {
            maxSets += _descCount;
            poolSizes.push_back({
                .type = _descType,
                .descriptorCount = _descCount
            });
        }

        const VkDescriptorPoolCreateInfo poolCreateInfo{
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
            log::debug("Destroyed Vulkan descriptor pool");
        }
    }

    void CVulkanDescriptorsPool::freeDescriptorSet(const VkDescriptorSet& descriptorSet) const {
        util::vkCheck(vkFreeDescriptorSets(this->_context.device().vkDevice(), this->_vkDescriptorPool, 1, &descriptorSet), "Failed to free descriptor set");
    }

    CVulkanDescriptorSetLayout::CVulkanDescriptorSetLayout(const CVulkanContext& context, const std::vector<CVulkanLayoutInfo>&& layoutInfoArray)
        : _context{context}, _layoutInfoArray{std::move(layoutInfoArray)} {
        log::debug("Creating Vulkan descriptor set layout with {} bindings", _layoutInfoArray.size());
        for (const auto& [_descType, _binding, _descCount, _stage]: _layoutInfoArray) {
            log::debug("Descriptor binding {}: type={}, count={}, stages={}", _binding, static_cast<uint32_t>(_descType), _descCount, _stage);
        }

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(layoutInfoArray.size());

        for (const auto& [_descType, _binding, _descCount, _stage]: layoutInfoArray) {
            layoutBindings.push_back({
                .binding = _binding,
                .descriptorType = _descType,
                .descriptorCount = _descCount,
                .stageFlags = _stage
            });
        }

        const VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
            .pBindings = layoutBindings.data()
        };
        util::vkCheck(vkCreateDescriptorSetLayout(context.device().vkDevice(), &vkDescriptorSetLayoutCreateInfo, nullptr, &this->_vkDescriptorSetLayout), "Failed to create descriptor set layout");
    }

    CVulkanDescriptorSetLayout::~CVulkanDescriptorSetLayout() {
        if (this->_vkDescriptorSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(this->_context.device().vkDevice(), this->_vkDescriptorSetLayout, nullptr);
            this->_vkDescriptorSetLayout = VK_NULL_HANDLE;
            log::debug("Destroyed Vulkan descriptor set layout");
        }
    }

    CVulkanDescriptorSet::CVulkanDescriptorSet(const CVulkanContext& context, const CVulkanDescriptorsPool& descriptorsPool, const CVulkanDescriptorSetLayout& vkDescriptorSetLayout)
        : _context{context}, _descriptorsPool{descriptorsPool} {
        const VkDescriptorSetLayout layout = vkDescriptorSetLayout.vkDescriptorSetLayout();

        const VkDescriptorSetAllocateInfo descriptorAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorsPool.vkDescriptorPool(),
            .descriptorSetCount = 1,
            .pSetLayouts = &layout
        };

        util::vkCheck(vkAllocateDescriptorSets(context.device().vkDevice(), &descriptorAllocateInfo, &this->_vkDescriptorSet), "Failed to create descriptor set");
    }

    CVulkanDescriptorSet::~CVulkanDescriptorSet() {
    }

    void CVulkanDescriptorSet::setBuffer(const CVulkanBuffer& buffer, const VkDeviceSize range, const uint32_t binding, const VkDescriptorType descriptorType) const {
        const VkDescriptorBufferInfo bufferInfo{
            .buffer = buffer.vkBuffer(),
            .offset = 0,
            .range = range,
        };

        const VkWriteDescriptorSet writeDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = this->_vkDescriptorSet,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = descriptorType,
            .pBufferInfo = &bufferInfo
        };

        vkUpdateDescriptorSets(this->_context.device().vkDevice(), 1, &writeDescriptorSet, 0, nullptr);
    }

    void CVulkanDescriptorSet::setImages(const std::span<const CVulkanImage2DView_CRef> imageViews, const CVulkanSampler& sampler, const uint32_t baseBinding) const {
        const uint32_t numImages = imageViews.size();
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(numImages);

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(imageViews.size());

        for (uint32_t i = 0; i < imageViews.size(); ++i) {
            const auto& imageView = imageViews[i];

            //TODO VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL
            imageInfos.push_back({
                .sampler = sampler.vkSampler(),
                .imageView = imageView.get().vkImageView(),
                .imageLayout = imageView.get().depthImage() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
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

    void CVulkanDescriptorSet::setImagesArray(const std::span<const CVulkanImage2DView_CRef> imageViews, const CVulkanSampler& sampler, const uint32_t baseBinding) const {
        const auto numImages = static_cast<uint32_t>(imageViews.size());

        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(numImages);

        for (const auto& imageView: imageViews) {
            imageInfos.push_back({
                .sampler = sampler.vkSampler(),
                .imageView = imageView.get().vkImageView(),
                .imageLayout = imageView.get().depthImage() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });
        }

        const VkWriteDescriptorSet write{
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
        log::info("Created Vulkan descriptor allocator");
    }

    CVulkanDescriptorAllocator::~CVulkanDescriptorAllocator() {
        log::info("Destroying Vulkan descriptor allocator");
    }

    const std::vector<CVulkanDescriptorSet>& CVulkanDescriptorAllocator::allocateDescriptorSets(std::string_view id, uint32_t count, const CVulkanDescriptorSetLayout& layout) {
        const std::string s_id{id};
        log::debug("Allocating descriptor sets '{}': count={}, bindings={}", id, count, layout.layoutInfoArray().size());

        DEBUG::ASSERT(count > 0, "Cannot allocate descriptor sets '{}': count must be greater than zero, received {}", id, count);
        DEBUG::ASSERT(!layout.layoutInfoArray().empty(), "Cannot allocate descriptor sets '{}': descriptor set layout has no bindings", id);
        DEBUG::ASSERT(!_descriptorSetInfoMap.contains(s_id), "Descriptor set with ID '{}' already exists", id);

        std::unordered_map<VkDescriptorType, uint64_t> requiredCounts{};

        for (const auto& layoutInfo : layout.layoutInfoArray()) {
            DEBUG::ASSERT(layoutInfo._descCount > 0, "Descriptor binding {} has zero descriptor count in layout '{}'", layoutInfo._binding, id);
            DEBUG::ASSERT(_vkDescriptorLimits.contains(layoutInfo._descType), "Descriptor type {} has no configured limit", static_cast<uint32_t>(layoutInfo._descType));

            const uint64_t requiredCount = static_cast<uint64_t>(layoutInfo._descCount) * count;
            requiredCounts[layoutInfo._descType] += requiredCount;

            const uint64_t maxTotal = _vkDescriptorLimits.at(layoutInfo._descType);
            DEBUG::ASSERT(requiredCounts.at(layoutInfo._descType) <= maxTotal, "Cannot allocate descriptor sets '{}': descriptor type {} requires {} descriptors, configured limit is {}", id, static_cast<uint32_t>(layoutInfo._descType), requiredCounts.at(layoutInfo._descType), maxTotal);
        }

        for (const auto& [descriptorType, requiredCount] : requiredCounts) {
            log::debug("Descriptor allocation requirement: type={}, count={}", static_cast<uint32_t>(descriptorType), requiredCount);
        }

        CVulkanDescriptorPoolInfo* targetPool{};
        uint32_t poolPos = 0;

        for (uint32_t i = 0; i < _descriptorPoolInfoList.size(); ++i) {
            CVulkanDescriptorPoolInfo& descriptorPoolInfo = _descriptorPoolInfoList[i];
            bool hasCapacity = true;

            for (const auto& [descriptorType, requiredCount] : requiredCounts) {
                if (!descriptorPoolInfo._descriptorCount.contains(descriptorType) || descriptorPoolInfo._descriptorCount.at(descriptorType) < requiredCount) {
                    const uint64_t available = descriptorPoolInfo._descriptorCount.contains(descriptorType) ? descriptorPoolInfo._descriptorCount.at(descriptorType) : 0;
                    log::trace("Descriptor pool {} lacks capacity for type {}: available={}, required={}", i, static_cast<uint32_t>(descriptorType), available, requiredCount);
                    hasCapacity = false;
                    break;
                }
            }

            if (hasCapacity) {
                targetPool = &descriptorPoolInfo;
                poolPos = i;
                log::debug("Selected descriptor pool {} for '{}'", poolPos, id);
                break;
            }
        }

        if (targetPool == nullptr) {
            log::debug("Creating new descriptor pool for '{}'", id);
            CVulkanDescriptorPoolInfo descriptorPoolInfo = createDescriptorPoolInfo();

            for (const auto& [descriptorType, requiredCount] : requiredCounts) {
                DEBUG::ASSERT(descriptorPoolInfo._descriptorCount.contains(descriptorType), "New descriptor pool does not support descriptor type {}", static_cast<uint32_t>(descriptorType));
                DEBUG::ASSERT(descriptorPoolInfo._descriptorCount.at(descriptorType) >= requiredCount, "New descriptor pool has insufficient capacity for type {}: available={}, required={}", static_cast<uint32_t>(descriptorType), descriptorPoolInfo._descriptorCount.at(descriptorType), requiredCount);
            }

            _descriptorPoolInfoList.push_back(std::move(descriptorPoolInfo));
            poolPos = static_cast<uint32_t>(_descriptorPoolInfoList.size() - 1);
            targetPool = &_descriptorPoolInfoList.back();
            log::debug("Created descriptor pool {} for '{}'", poolPos, id);
        }

        std::vector<CVulkanDescriptorSet> result{};
        result.reserve(count);

        log::debug("Creating {} descriptor sets '{}' from pool {}", count, id, poolPos);
        for (uint32_t i = 0; i < static_cast<uint32_t>(count); ++i) {
            log::trace("Allocating descriptor set '{}': {}/{}", id, i + 1, count);
            CVulkanDescriptorSet descriptorSet{_context, targetPool->_descriptorsPool, layout};
            result.push_back(std::move(descriptorSet));
        }

        const auto [it, inserted] = _descriptorSetInfoMap.emplace(s_id, CVulkanDescriptorSetInfo{std::move(result), poolPos});
        DEBUG::ASSERT(inserted, "Failed to register descriptor sets '{}': ID already exists", id);

        for (const auto& [descriptorType, requiredCount] : requiredCounts) {
            targetPool->_descriptorCount.at(descriptorType) -= requiredCount;
            log::debug("Descriptor pool {} remaining capacity: type={}, remaining={}", poolPos, static_cast<uint32_t>(descriptorType), targetPool->_descriptorCount.at(descriptorType));
        }

        log::debug("Descriptor sets '{}' allocated successfully: count={}, pool={}", id, count, poolPos);
        return it->second._descriptorSets;
    }

    void CVulkanDescriptorAllocator::createDescriptorLimits() {
        const VkPhysicalDeviceLimits& limits = _context.physicalDevice().vkPhysicalDeviceProperties().limits;
        this->_vkDescriptorLimits.emplace(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, std::min(renderConfig::MAX_DESCRIPTORS, limits.maxDescriptorSetUniformBuffers));
        this->_vkDescriptorLimits.emplace(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, std::min(renderConfig::MAX_DESCRIPTORS, limits.maxDescriptorSetSamplers));
        this->_vkDescriptorLimits.emplace(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, std::min(renderConfig::MAX_DESCRIPTORS, limits.maxDescriptorSetStorageBuffers));
    }

    CVulkanDescriptorPoolInfo CVulkanDescriptorAllocator::createDescriptorPoolInfo() {
        std::vector<CVulkanDescriptorTypeCount> descTypeCounts;
        descTypeCounts.reserve(this->_vkDescriptorLimits.size());
        for (const auto& [type, count]: this->_vkDescriptorLimits) {
            descTypeCounts.push_back({type, count});
        }
        return {this->_vkDescriptorLimits, {this->_context, descTypeCounts}};
    }

    void CVulkanDescriptorAllocator::freeDescriptorSet(const std::string_view id) {
        const std::string s_id{id};

        const auto it = _descriptorSetInfoMap.find(s_id);
        DEBUG::ASSERT(it != _descriptorSetInfoMap.end(), "Descriptor set with ID '{}' not found", id);

        const CVulkanDescriptorSetInfo& descriptorSetInfo = it->second;
        DEBUG::ASSERT(descriptorSetInfo._poolPos < _descriptorPoolInfoList.size(), "No descriptor pool associated with descriptor set '{}': pool position={}, available pools={}", id, descriptorSetInfo._poolPos, _descriptorPoolInfoList.size());

        CVulkanDescriptorPoolInfo& descriptorPoolInfo = _descriptorPoolInfoList.at(descriptorSetInfo._poolPos);

        for (const auto& descriptorSet : descriptorSetInfo._descriptorSets) {
            descriptorPoolInfo._descriptorsPool.freeDescriptorSet(descriptorSet.vkDescriptorSet());
        }

        _descriptorSetInfoMap.erase(it);
    }


    const CVulkanDescriptorSet& CVulkanDescriptorAllocator::getDescriptorSet(const std::string_view id, const uint32_t pos) const {
        const std::string s_id{id};

        const auto it = _descriptorSetInfoMap.find(s_id);
        DEBUG::ASSERT(it != _descriptorSetInfoMap.end(), "Descriptor set with ID '{}' not found", id);

        const auto& descriptorSets = it->second._descriptorSets;
        DEBUG::ASSERT(pos < descriptorSets.size(), "Descriptor set position {} is out of range for ID '{}': available sets={}", pos, id, descriptorSets.size());

        return descriptorSets.at(pos);
    }
}
