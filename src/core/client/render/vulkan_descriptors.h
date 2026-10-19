//
// Created by ded on 12.10.2026.
//

#pragma once
#include "vulkan_utility.h"

namespace cvulkan::client::render::vk {
    class CVulkanDescriptorSet;
    class CVulkanBuffer;
    class CVulkanSampler;
    class CVulkanImage2DView;
    class CVulkanContext;
    class CVulkanDescriptorsPool;

    struct CVulkanDescriptorTypeCount {
        VkDescriptorType _descType;
        uint32_t _descCount;
    };

    struct CVulkanLayoutInfo {
        VkDescriptorType _descType;
        uint32_t _binding;
        uint32_t _descCount;
        VkShaderStageFlags _stage;
    };

    struct CVulkanDescriptorSetInfo {
        std::vector<CVulkanDescriptorSet> _descriptorSets;
        uint32_t _poolPos;
    };

    class CVulkanDescriptorsPool {
    public:
        CVulkanDescriptorsPool(const CVulkanContext& context, const std::vector<CVulkanDescriptorTypeCount>& descriptorTypeCounts);
        ~CVulkanDescriptorsPool();

        CVULKAN_NO_COPY_NO_ASSIGN_MOVE(CVulkanDescriptorsPool)
        CVulkanDescriptorsPool(CVulkanDescriptorsPool&& other) noexcept : _context(other._context), _descriptorTypeCounts(std::move(other._descriptorTypeCounts)), _vkDescriptorPool(other._vkDescriptorPool) {
            other._vkDescriptorPool = VK_NULL_HANDLE;
        };

        void freeDescriptorSet(const VkDescriptorSet& descriptorSet) const;

        [[nodiscard]] const std::vector<CVulkanDescriptorTypeCount>& descriptorTypeCounts() const {
            return _descriptorTypeCounts;
        }

        [[nodiscard]] VkDescriptorPool vkDescriptorPool() const {
            return _vkDescriptorPool;
        }

    private:
        const CVulkanContext& _context;
        std::vector<CVulkanDescriptorTypeCount> _descriptorTypeCounts;
        VkDescriptorPool _vkDescriptorPool{};
    };


    struct CVulkanDescriptorPoolInfo {
        std::unordered_map<VkDescriptorType, uint32_t> _descriptorCount;
        CVulkanDescriptorsPool _descriptorsPool;
    };


    class CVulkanDescriptorSetLayout {
    public:
        CVulkanDescriptorSetLayout(const CVulkanContext& context, const std::vector<CVulkanLayoutInfo>&& layoutInfoArray);
        ~CVulkanDescriptorSetLayout();

        CVULKAN_NO_COPY_NO_ASSIGN_MOVE(CVulkanDescriptorSetLayout)
        CVulkanDescriptorSetLayout(CVulkanDescriptorSetLayout&& other) noexcept : _context(other._context), _layoutInfoArray(std::move(other._layoutInfoArray)), _vkDescriptorSetLayout(other._vkDescriptorSetLayout) {
            other._vkDescriptorSetLayout = VK_NULL_HANDLE;
        }

        [[nodiscard]] const std::vector<CVulkanLayoutInfo>& layoutInfoArray() const {
            return _layoutInfoArray;
        }

        [[nodiscard]] VkDescriptorSetLayout vkDescriptorSetLayout() const {
            return _vkDescriptorSetLayout;
        }

    private:
        const CVulkanContext& _context;
        std::vector<CVulkanLayoutInfo> _layoutInfoArray;
        VkDescriptorSetLayout _vkDescriptorSetLayout{};
    };

    class CVulkanDescriptorSet {
    public:
        CVulkanDescriptorSet(const CVulkanContext& context, const CVulkanDescriptorsPool& descriptorsPool, const CVulkanDescriptorSetLayout& vkDescriptorSetLayout);
        ~CVulkanDescriptorSet();

        CVULKAN_NO_COPY_NO_ASSIGN_MOVE(CVulkanDescriptorSet)
        CVulkanDescriptorSet(CVulkanDescriptorSet&& other) noexcept : _context(other._context), _descriptorsPool(other._descriptorsPool), _vkDescriptorSet(other._vkDescriptorSet) {
            other._vkDescriptorSet = VK_NULL_HANDLE;
        }

        void setBuffer(const CVulkanBuffer& buffer, VkDeviceSize range, uint32_t binding, VkDescriptorType descriptorType) const;

        void setImages(const std::vector<const CVulkanImage2DView*>& imageViews, const CVulkanSampler& sampler, uint32_t baseBinding) const;
        void setImagesArray(const std::vector<const CVulkanImage2DView*>& imageViews, const CVulkanSampler& sampler, uint32_t baseBinding) const;

        [[nodiscard]] VkDescriptorSet vkDescriptorSet() const {
            return _vkDescriptorSet;
        }

    private:
        const CVulkanContext& _context;
        const CVulkanDescriptorsPool& _descriptorsPool;
        VkDescriptorSet _vkDescriptorSet{};
    };

    class CVulkanDescriptorAllocator {
    public:
        explicit CVulkanDescriptorAllocator(const CVulkanContext& context);
        ~CVulkanDescriptorAllocator();

        CVULKAN_NO_COPY_NO_MOVE(CVulkanDescriptorAllocator)

        std::vector<CVulkanDescriptorSet>& allocateDescriptorSets(const std::string& id, uint32_t count, const CVulkanDescriptorSetLayout& layout);

        void freeDescriptorSet(const std::string& id);

        const CVulkanDescriptorSet& getDescriptorSet(const std::string& id, uint32_t pos) const;

        [[nodiscard]] const std::unordered_map<VkDescriptorType, uint32_t>& descriptorLimits() const {
            return _descriptorLimits;
        }

        [[nodiscard]] const std::unordered_map<std::string, CVulkanDescriptorSetInfo>& descriptorSetInfoMap() const {
            return _descriptorSetInfoMap;
        }

        [[nodiscard]] const std::vector<CVulkanDescriptorPoolInfo>& descriptorPoolInfoList() const {
            return _descriptorPoolInfoList;
        }

    protected:
        void createDescriptorLimits();
        CVulkanDescriptorPoolInfo createDescriptorPoolInfo();

    private:
        const CVulkanContext& _context;
        std::unordered_map<VkDescriptorType, uint32_t> _descriptorLimits{};
        std::unordered_map<std::string, CVulkanDescriptorSetInfo> _descriptorSetInfoMap{};
        std::vector<CVulkanDescriptorPoolInfo> _descriptorPoolInfoList{};
    };
}