//
// Created by ded on 04.11.2026.
//
#pragma once

#include "utility/logger.h"

namespace cvulkan::client::render::vk {
    template<ValidSpecConst... Args>
    CVulkanSpecConstants<Args...>::CVulkanSpecConstants(Args... args) {
        uint32_t offset = 0;
        uint32_t i = 0;

        const auto entry = [&]<typename T0>(const T0& value) {
            using T = std::decay_t<T0>;

            using StorageType = SpecConstType<T>;
            StorageType convertedValue = static_cast<StorageType>(value);

            this->_entries[i] = {
                .constantID = i,
                .offset = offset,
                .size = sizeof(StorageType)
            };

            const auto* bytes = reinterpret_cast<const char*>(&convertedValue);
            this->_data.insert(this->_data.end(), bytes, bytes + sizeof(StorageType));

            offset += sizeof(StorageType);
            i++;
        };

        (entry(args), ...);

        this->_specializationInfo = {
            .mapEntryCount = this->_entries.size(),
            .pMapEntries = this->_entries.data(),
            .dataSize = this->_data.size(),
            .pData = this->_data.data()
        };
        logging::debug("Created specialization constant");
    }

    template<ValidSpecConst ... Args>
    CVulkanSpecConstants<Args...>::~CVulkanSpecConstants() {
        logging::debug("Destroyed specialization constant");
    }
}
