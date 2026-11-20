//
// Created by ded on 12.10.2026.
//

#pragma once
#include "core/utility/system.h"
#include <span>

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

    using CVulkanImage2DView_CRef = std::reference_wrapper<const CVulkanImage2DView>;

    class CVulkanDescriptorsPool {
    public:
        CVulkanDescriptorsPool(const CVulkanContext& context, const std::vector<CVulkanDescriptorTypeCount>& descriptorTypeCounts);

        ~CVulkanDescriptorsPool();

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanDescriptorsPool)

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
        //CVulkanDescriptorSetLayout(const CVulkanContext& context, const std::initializer_list<CVulkanLayoutInfo> layoutInfoArray)
        //    : CVulkanDescriptorSetLayout(context, std::vector(layoutInfoArray.begin(), layoutInfoArray.end())) {}

        CVulkanDescriptorSetLayout(const CVulkanContext& context, const std::vector<CVulkanLayoutInfo>&& layoutInfoArray);

        CVulkanDescriptorSetLayout(const CVulkanContext& context, CVulkanLayoutInfo&& layoutInfo)
            : CVulkanDescriptorSetLayout(context, std::vector{std::move(layoutInfo)}) {
        }

        ~CVulkanDescriptorSetLayout();

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanDescriptorSetLayout)

        CVulkanDescriptorSetLayout(CVulkanDescriptorSetLayout&& other) noexcept : _context(other._context), _layoutInfoArray(std::move(other._layoutInfoArray)), _vkDescriptorSetLayout(other._vkDescriptorSetLayout) {
            other._vkDescriptorSetLayout = VK_NULL_HANDLE;
        }

        [[nodiscard]] const CVulkanLayoutInfo& layoutInfo() const {
            return _layoutInfoArray.at(0);
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

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanDescriptorSet)

        CVulkanDescriptorSet(CVulkanDescriptorSet&& other) noexcept : _context(other._context), _descriptorsPool(other._descriptorsPool), _vkDescriptorSet(other._vkDescriptorSet) {
            other._vkDescriptorSet = VK_NULL_HANDLE;
        }

        void setBuffer(const CVulkanBuffer& buffer, VkDeviceSize range, uint32_t binding, VkDescriptorType descriptorType) const;

        void setImage(const CVulkanImage2DView& imageView, const CVulkanSampler& sampler) const {
            this->setImage(imageView, sampler, 0);
        }

        void setImage(const CVulkanImage2DView& imageView, const CVulkanSampler& sampler, const uint32_t baseBinding) const {
            const std::array imageViews = {std::cref(imageView)};
            this->setImages(imageViews, sampler, baseBinding);
        }

        void setImages(std::span<const CVulkanImage2DView_CRef> imageViews, const CVulkanSampler& sampler, uint32_t baseBinding) const;

        void setImagesArray(std::span<const CVulkanImage2DView_CRef> imageViews, const CVulkanSampler& sampler, uint32_t baseBinding) const;

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

        CLASS_NO_COPY_NO_MOVE(CVulkanDescriptorAllocator)

        const std::vector<CVulkanDescriptorSet>& allocateDescriptorSets(std::string_view id, uint32_t count, const CVulkanDescriptorSetLayout& layout);

        const CVulkanDescriptorSet& allocateDescriptorSet(const std::string_view id, const CVulkanDescriptorSetLayout& layout) {
            return this->allocateDescriptorSets(id, 1, layout).front();
        }

        void freeDescriptorSet(std::string_view id);

        const CVulkanDescriptorSet& getDescriptorSet(std::string_view id, uint32_t pos) const;

        [[nodiscard]] const std::unordered_map<VkDescriptorType, uint32_t>& vkDescriptorLimits() const {
            return _vkDescriptorLimits;
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
        std::unordered_map<VkDescriptorType, uint32_t> _vkDescriptorLimits{};
        std::unordered_map<std::string, CVulkanDescriptorSetInfo> _descriptorSetInfoMap{};
        std::vector<CVulkanDescriptorPoolInfo> _descriptorPoolInfoList{};
    };
}
