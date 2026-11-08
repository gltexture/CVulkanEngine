//
// Created by ded on 04.11.2026.
//

#pragma once
#include "utility/sys_utils.h"

namespace cvulkan::client::render::vk {
    template <typename T>
    using SpecConstType = std::conditional_t<
        std::is_same_v<std::decay_t<T>, bool>, uint32_t, std::decay_t<T>
    >;

    template<typename T>
    concept ValidSpecConst =
        std::is_same_v<T, int32_t> ||
        std::is_same_v<T, uint32_t> ||
        std::is_same_v<T, int64_t> ||
        std::is_same_v<T, uint64_t> ||
        std::is_same_v<T, float> ||
        std::is_same_v<T, double> ||
        std::is_same_v<T, bool>;

    template<ValidSpecConst... Args>
    class CVulkanSpecConstants {
    public:
        explicit CVulkanSpecConstants(Args... args);
        ~CVulkanSpecConstants();

        CLASS_NO_COPY(CVulkanSpecConstants)
        CVulkanSpecConstants& operator=(CVulkanSpecConstants&&) = default;
        CVulkanSpecConstants(CVulkanSpecConstants&& other) = default;

        [[nodiscard]] const VkSpecializationInfo& specializationInfo() const {
            return _specializationInfo;
        }

    private:
        std::vector<char> _data {};
        std::array<VkSpecializationMapEntry, sizeof...(Args)> _entries {};
        VkSpecializationInfo _specializationInfo{};
    };
}

#include "vulkan_spec_const.tpp"