//
// Created by ded on 14.09.2026.
//

#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "vulkan_render_structs.h"
#include "client/render/vulkan_commands.h"

namespace cvulkan::client::render::core {
    class CVulkanCommandBuffer;
    class CVulkanContext;
}

namespace cvulkan::client::render::cache {
    class CVulkanModelsCache {
    public:
        explicit CVulkanModelsCache(const core::CVulkanContext& context)
            : _context(context) {
        }
        ~CVulkanModelsCache();

        CVULKAN_NO_COPY(CVulkanModelsCache);

        void loadModels(const std::vector<structs::CVulkanRawModelData>& models, const core::CVulkanCommandPool& commandPool, const core::CVulkanQueue& queue);

        [[nodiscard]] const structs::CVulkanModel& getModel(std::string_view id);

        [[nodiscard]] const std::unordered_map<std::string, structs::CVulkanModel>& models() const {
            return _modelsCache;
        }

    private:
        const core::CVulkanContext& _context;
        std::unordered_map<std::string, structs::CVulkanModel> _modelsCache;
    };

    class CVulkanCacheCollection {
    public:
        explicit CVulkanCacheCollection(const core::CVulkanContext& context)
            : _context(context), _modelCache{context} {}

        [[nodiscard]] CVulkanModelsCache& modelCache() {
            return _modelCache;
        }

        [[nodiscard]] const CVulkanModelsCache& modelCache() const {
            return _modelCache;
        }

    private:
        const core::CVulkanContext& _context;
        CVulkanModelsCache _modelCache;
    };
}
