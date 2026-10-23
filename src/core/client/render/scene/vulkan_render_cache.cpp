//
// Created by ded on 14.09.2026.
//

#include "vulkan_render_cache.h"

#include <ranges>

#include "client/render/vulkan_config.hpp"
#include "client/render/assets/asset_models.h"

namespace cvulkan::client::render::cache {
    CVulkanModelsCache::~CVulkanModelsCache() {
        logging::debug("Cleaned up models cache");
    }

    void CVulkanModelsCache::loadModels(const std::vector<asset::modelGen::CVulkanModelData>& models, const vk::CVulkanCommandPool& commandPool, const vk::CVulkanQueue& queue) {
        std::vector<vk::CVulkanBuffer> stagingBuffers{};

        const vk::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        commandBuffer.beginRecording();

        for (const auto& [id, meshes, _materialsPath_Relative, _cachePath_Relative, _idxPath_Relative, _vtxPath_Relative] : models) {
            logging::trace("Loading model {}", id);
            this->_modelsMap.emplace(id, structs::CVulkanModel{id});
            for (const auto& mesh : meshes) {
                vk::CVulkanTransferBufferData verticesBuffers = vk::createVerticesBuffer(this->_context, mesh, io_utils::RES_RESOLVER / (_vtxPath_Relative));
                vk::CVulkanTransferBufferData indicesBuffer = vk::createIndicesBuffers(this->_context, mesh, io_utils::RES_RESOLVER / (_idxPath_Relative));

                verticesBuffers.recordTransferCommand(commandBuffer);
                indicesBuffer.recordTransferCommand(commandBuffer);

                stagingBuffers.emplace_back(std::move(verticesBuffers._src));
                stagingBuffers.emplace_back(std::move(indicesBuffer._src));

                structs::CVulkanMesh vulkanMesh{mesh._id, std::move(verticesBuffers._dst), std::move(indicesBuffer._dst), static_cast<uint32_t>(mesh._idxSize / sizeof(uint32_t)), mesh._materialId};
                this->_modelsMap.at(id).meshes().emplace_back(std::move(vulkanMesh));
            }
        }

        commandBuffer.endRecording();
        commandBuffer.submitAndWait(queue);
    }

    const structs::CVulkanModel* CVulkanModelsCache::getModel(const std::string_view id) const {
        const auto it = this->_modelsMap.find(std::string{id});
        if (it == this->_modelsMap.end()) {
            return nullptr;
        }
        return &it->second;
    }

    CVulkanTexturesCache::~CVulkanTexturesCache() {
        logging::debug("Cleaned up textures cache");
    }

    void CVulkanTexturesCache::transitTextures(const vk::CVulkanCommandPool& commandPool, const vk::CVulkanQueue& queue) {
        static std::uint64_t counter = 0;

        logging::debug("Transiting textures cache...");
        if (const uint32_t numTextures = this->_indexedTexturesMap.map().size(); numTextures < renderConfig::MAX_TEXTURES_IN_CACHE) {
            for (int i = 0; i < renderConfig::MAX_TEXTURES_IN_CACHE - numTextures; i++) {
                this->loadTexture(std::format("default_texture_{}", std::to_string(counter++)), io_utils::RES_RESOLVER / io_utils::CVulkanResourcePath{io_utils::ERROR_TEXTURE}, VK_FORMAT_R8G8B8A8_SRGB);
            }
        }
        const vk::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        commandBuffer.beginRecording();
        for (auto& v : this->_indexedTexturesMap.map() | std::views::values) {
            v.recordTextureTransition(commandBuffer);
        }
        commandBuffer.endRecording();
        commandBuffer.submitAndWait(queue);
        for (auto& v : this->_indexedTexturesMap.map() | std::views::values) {
            v.destroyStgBuffer();
        }
        logging::debug("Transition complete");
    }

    const asset::CVulkanTexture2D& CVulkanTexturesCache::loadTexture(const std::string& id, const asset::CVulkanImageSrc& srcImage, const VkFormat& format) {
        if (this->_indexedTexturesMap.map().size() >= renderConfig::MAX_TEXTURES_IN_CACHE) {
            throw std::runtime_error("Texture cache is full");
        }
        if (const auto it = this->_indexedTexturesMap.map().find(id); it != this->_indexedTexturesMap.map().end()) {
            return it->second;
        }
        auto texture = asset::CVulkanTexture2D {this->_context, id, srcImage, format};
        this->_indexedTexturesMap.emplace(id, std::move(texture));
        return this->_indexedTexturesMap.getValueAtIndex(this->_indexedTexturesMap.map().size() - 1);
    }

    const asset::CVulkanTexture2D* CVulkanTexturesCache::loadTexture(const std::string& id, const std::filesystem::path& texturePath, const VkFormat& format) {
        if (const auto it = this->_indexedTexturesMap.map().find(id); it != this->_indexedTexturesMap.map().end()) {
            return &it->second;
        }
        const asset::CVulkanImageSrc srcImage = asset::imageLoad::loadImage(texturePath);
        const asset::CVulkanTexture2D& texture = this->loadTexture(id, srcImage, format);
        return &texture;
    }

    [[nodiscard]] int32_t CVulkanTexturesCache::getPosition(const std::string& id) const {
        const auto res = this->_indexedTexturesMap.getIndexOf(id);
        if (res < 0) {
            return -1;
        }
        return static_cast<uint32_t>(res);
    }

    CVulkanMaterialsCache::~CVulkanMaterialsCache() {
        logging::debug("Cleaned up materials cache");
    }

    void CVulkanMaterialsCache::loadMaterials(const std::vector<asset::modelGen::CVulkanMaterialData>& materials, CVulkanTexturesCache& texturesCache, const vk::CVulkanCommandPool& commandPool, const vk::CVulkanQueue& queue) {
        const uint32_t numMaterials = materials.size();
        const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(MAT_BUFFER_SIZE) * materials.size();

        vk::CVulkanBuffer srcBuffer {this->_context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize};
        vk::CVulkanBuffer matBuffer {this->_context, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize};

        const vk::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        commandBuffer.beginRecording();
        vk::CVulkanTransferBufferData transferBufferData {
            ._src = std::move(srcBuffer),
            ._dst = std::move(matBuffer),
        };

        transferBufferData._src.mapMem();
        uint32_t offset = 0;
        for (uint32_t i = 0; i < numMaterials; i++) {
            const auto& [_id, _texturePath, _diffuseColor] = materials.at(i);
            int32_t texturePos = -1;
            int32_t hasTextureValue32 = 0;

            if (!_texturePath.isEmpty()) {
                if (const std::filesystem::path texturePath = io_utils::RES_RESOLVER / _texturePath; std::filesystem::is_regular_file(texturePath)) {
                    texturesCache.loadTexture(texturePath.string(), texturePath, VK_FORMAT_R8G8B8A8_SRGB);
                    texturePos = texturesCache.getPosition(texturePath.string());
                    hasTextureValue32 = 1;
                }
            }

            const structs::CVulkanMaterial material {_id};
            this->_indexedMaterialsMap.emplace(material._id, material);

            auto* mappedMemory = static_cast<char*>(transferBufferData._src.mappedMemory());
            std::memcpy(mappedMemory + offset, &_diffuseColor, sizeof(glm::vec4)); offset += sizeof(glm::vec4);
            std::memcpy(mappedMemory + offset, &hasTextureValue32, sizeof(hasTextureValue32)); offset += sizeof(hasTextureValue32);
            std::memcpy(mappedMemory + offset, &texturePos, sizeof(texturePos)); offset += sizeof(texturePos);

            { // PADDING
                constexpr uint32_t padding = 0;
                std::memcpy(mappedMemory + offset, &padding, sizeof(padding)); offset += sizeof(padding);
                std::memcpy(mappedMemory + offset, &padding, sizeof(padding)); offset += sizeof(padding);
            }
        }
        transferBufferData._src.unMapMem();
        transferBufferData.recordTransferCommand(commandBuffer);

        commandBuffer.endRecording();
        commandBuffer.submitAndWait(queue);

        this->_materialsBuffer = std::make_unique<vk::CVulkanBuffer>(std::move(transferBufferData._dst));
    }

    const structs::CVulkanMaterial* CVulkanMaterialsCache::getMaterial(const std::string& id) const {
        const auto it = this->_indexedMaterialsMap.map().find(id);
        if (it == this->_indexedMaterialsMap.map().end()) {
            return nullptr;
        }
        return &it->second;
    }

    uint32_t CVulkanMaterialsCache::getPosition(const std::string& id) const {
        const auto res = this->_indexedMaterialsMap.getIndexOf(id);
        if (res < 0) {
            throw std::runtime_error(std::format("Could not find material with id={}", id));
        }
        return static_cast<uint32_t>(res);
    }
}
