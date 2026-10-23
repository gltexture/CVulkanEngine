//
// Created by ded on 14.09.2026.
//

#pragma once

#include <string>
#include <unordered_map>

#include "vulkan_render_structs.h"
#include "client/render/vulkan_commands.h"
#include "../../../utility/collections.hpp"
#include "client/render/assets/asset_textures.h"

namespace cvulkan::client::render::asset {
    namespace modelGen {
        struct CVulkanMaterialData;
        struct CVulkanModelData;
    }

    struct CVulkanImageSrc;

}

namespace cvulkan::client::render::vk {
    class CVulkanCommandBuffer;
    class CVulkanContext;
}

namespace cvulkan::client::render::cache {
    inline constexpr uint32_t MAT_BUFFER_SIZE = sizeof(glm::vec4) + sizeof(uint32_t) * 4;

    class CVulkanModelsCache {
    public:
        explicit CVulkanModelsCache(const vk::CVulkanContext& context)
            : _context{context} { }
        ~CVulkanModelsCache();

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanModelsCache)

        void loadModels(const std::vector<asset::modelGen::CVulkanModelData>& models, const vk::CVulkanCommandPool& commandPool, const vk::CVulkanQueue& queue);

        [[nodiscard]] const structs::CVulkanModel* getModel(std::string_view id) const;

        [[nodiscard]] const std::unordered_map<std::string, structs::CVulkanModel>& modelsMap() const {
            return _modelsMap;
        }

    private:
        const vk::CVulkanContext& _context;
        std::unordered_map<std::string, structs::CVulkanModel> _modelsMap;
    };

    class CVulkanTexturesCache {
    public:
        explicit CVulkanTexturesCache(const vk::CVulkanContext& context)
            : _context{context} { }
        ~CVulkanTexturesCache();

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanTexturesCache)

        void transitTextures(const vk::CVulkanCommandPool& commandPool, const vk::CVulkanQueue& queue);

        const asset::CVulkanTexture2D& loadTexture(const std::string &id, const asset::CVulkanImageSrc &srcImage, const VkFormat &format);

        const asset::CVulkanTexture2D* loadTexture(const std::string& id, const std::filesystem::path& texturePath, const VkFormat& format);

        [[nodiscard]] const collection::IndexedMap<std::string, asset::CVulkanTexture2D>& indexedTexturesMap() const {
            return _indexedTexturesMap;
        }

        [[nodiscard]] int32_t getPosition(const std::string& id) const;

    private:
        const vk::CVulkanContext& _context;
        collection::IndexedMap<std::string, asset::CVulkanTexture2D> _indexedTexturesMap{};
    };


    class CVulkanMaterialsCache {
    public:
        explicit CVulkanMaterialsCache(const vk::CVulkanContext& context)
            : _context{context} { }
        ~CVulkanMaterialsCache();

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanMaterialsCache)

        void loadMaterials(const std::vector<asset::modelGen::CVulkanMaterialData>& materials, CVulkanTexturesCache& texturesCache, const vk::CVulkanCommandPool& commandPool, const vk::CVulkanQueue& queue);

        [[nodiscard]] const structs::CVulkanMaterial* getMaterial(const std::string& id) const;

        [[nodiscard]] const collection::IndexedMap<std::string, structs::CVulkanMaterial>& indexedMaterialsMap() const {
            return _indexedMaterialsMap;
        }

        [[nodiscard]] const vk::CVulkanBuffer* materialsBuffer() const {
            return _materialsBuffer.get();
        }

        [[nodiscard]] uint32_t getPosition(const std::string& id) const;

    private:
        const vk::CVulkanContext& _context;
        collection::IndexedMap<std::string, structs::CVulkanMaterial> _indexedMaterialsMap{};
        std::unique_ptr<vk::CVulkanBuffer> _materialsBuffer;
    };



    class CVulkanCacheCollection {
    public:
        explicit CVulkanCacheCollection(const vk::CVulkanContext& context)
            : _context{context}, _modelCache{context}, _texturesCache{context}, _materialsCache{context} { }

        CLASS_NO_COPY_NO_ASSIGN_MOVE(CVulkanCacheCollection)

        [[nodiscard]] CVulkanModelsCache& modelCache() {
            return _modelCache;
        }

        [[nodiscard]] const CVulkanModelsCache& modelCache() const {
            return _modelCache;
        }

        [[nodiscard]] CVulkanTexturesCache& texturesCache() {
            return _texturesCache;
        }

        [[nodiscard]] const CVulkanTexturesCache& texturesCache() const {
            return _texturesCache;
        }

        [[nodiscard]] CVulkanMaterialsCache& materialsCache() {
            return _materialsCache;
        }

        [[nodiscard]] const CVulkanMaterialsCache& materialsCache() const {
            return _materialsCache;
        }

    private:
        const vk::CVulkanContext& _context;
        CVulkanModelsCache _modelCache;
        CVulkanTexturesCache _texturesCache;
        CVulkanMaterialsCache _materialsCache;
    };
}
