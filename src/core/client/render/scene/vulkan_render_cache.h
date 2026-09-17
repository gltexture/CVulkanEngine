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
            : _context(context) {}
        ~CVulkanModelsCache() = default;

        CVULKAN_NO_COPY(CVulkanModelsCache);

        void loadModels(const std::vector<structs::CVulkanRawModelData>& models, const core::CVulkanCommandPool& commandPool, const core::CVulkanQueue& queue);
        void destroyCache();

        [[nodiscard]] const structs::CVulkanModel& getModel(const std::string_view id);

        [[nodiscard]] const std::unordered_map<std::string, structs::CVulkanModel>& models() const {
            return _modelsCache;
        }

    private:
        const core::CVulkanContext& _context;
        std::unordered_map<std::string, structs::CVulkanModel> _modelsCache;
    };

    extern std::unique_ptr<CVulkanModelsCache> modelsCache;

    void createCaches(const core::CVulkanContext& context);
    void destroyCaches();
}
