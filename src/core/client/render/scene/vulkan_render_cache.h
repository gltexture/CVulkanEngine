//
// Created by ded on 14.09.2026.
//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "vulkan_render_structs.h"
#include "client/render/vulkan_commands.h"
#include "util/collections.hpp"

namespace cvulkan::client::render::asset {
    struct CVulkanImageSrc;
    class CVulkanTexture2D;
}

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

        [[nodiscard]] const structs::CVulkanModel* getModel(std::string_view id) const;

        [[nodiscard]] const std::unordered_map<std::string, structs::CVulkanModel>& models() const {
            return _modelsCache;
        }

    private:
        const core::CVulkanContext& _context;
        std::unordered_map<std::string, structs::CVulkanModel> _modelsCache;
    };

    class CVulkanTexturesCache {
    public:
        explicit CVulkanTexturesCache(const core::CVulkanContext& context)
            : _context(context) {
        }
        ~CVulkanTexturesCache();

        CVULKAN_NO_COPY(CVulkanTexturesCache);

        void transitTextures(const core::CVulkanCommandPool& commandPool, const core::CVulkanQueue& queue);

        const asset::CVulkanTexture2D& loadTexture(const std::string& id, const asset::CVulkanImageSrc& srcImage, const VkFormat& format);

        const asset::CVulkanTexture2D* loadTexture(const std::string& id, const std::filesystem::path& texturePath, const VkFormat& format);

        [[nodiscard]] const collection::IndexedMap<std::string, asset::CVulkanTexture2D>& map() const {
            return _indexedMap;
        }

        [[nodiscard]] int getPosition(const std::string* id) const;

    private:
        const core::CVulkanContext& _context;
        collection::IndexedMap<std::string, asset::CVulkanTexture2D> _indexedMap{};
    };


    class CVulkanCacheCollection {
    public:
        explicit CVulkanCacheCollection(const core::CVulkanContext& context)
            : _context(context), _modelCache{context}, _texturesCache{context} {}

        [[nodiscard]] CVulkanModelsCache& modelCache() {
            return _modelCache;
        }

        [[nodiscard]] const CVulkanModelsCache& modelCache() const {
            return _modelCache;
        }

        [[nodiscard]] const CVulkanTexturesCache& texturesCache() const {
            return _texturesCache;
        }

    private:
        const core::CVulkanContext& _context;
        CVulkanModelsCache _modelCache;
        CVulkanTexturesCache _texturesCache;
    };
}
