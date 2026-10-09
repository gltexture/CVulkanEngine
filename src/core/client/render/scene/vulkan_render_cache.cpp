//
// Created by ded on 14.09.2026.
//

#include "vulkan_render_cache.h"

#include <ranges>

#include "client/render/vulkan_config.h"
#include "client/render/assets/asset_textures.h"

namespace cvulkan::client::render::cache {
    CVulkanModelsCache::~CVulkanModelsCache() {
        logging::debug("Cleaned up models cache");
    }

    void CVulkanModelsCache::loadModels(const std::vector<structs::CVulkanRawModelData>& models, const core::CVulkanCommandPool& commandPool, const core::CVulkanQueue& queue) {
        std::vector<core::CVulkanBuffer> stagingBuffers{};

        const core::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        commandBuffer.beginRecording();

        for (const auto& [id, meshes]: models) {
            logging::trace("Loading model {}", id);
            this->_modelsCache.emplace(id, structs::CVulkanModel{id});
            for (const auto& mesh: meshes) {
                core::CVulkanTransferBufferData verticesBuffers = core::createVerticesBuffer(this->_context, mesh);
                core::CVulkanTransferBufferData indicesBuffer = core::createIndicesBuffers(this->_context, mesh);

                verticesBuffers.recordTransferCommand(commandBuffer);
                indicesBuffer.recordTransferCommand(commandBuffer);

                stagingBuffers.emplace_back(std::move(verticesBuffers.src));
                stagingBuffers.emplace_back(std::move(indicesBuffer.src));

                structs::CVulkanMesh vulkanMesh{id, std::move(verticesBuffers.dst), std::move(indicesBuffer.dst), static_cast<uint32_t>(mesh.indices.size())};
                this->_modelsCache.at(id).meshes().emplace_back(std::move(vulkanMesh));
            }
        }

        commandBuffer.endRecording();
        commandBuffer.submitAndWait(queue);
    }

    const structs::CVulkanModel* CVulkanModelsCache::getModel(const std::string_view id) const {
        const auto it = this->_modelsCache.find(std::string{id});
        if (it == this->_modelsCache.end()) {
            return nullptr;
        }
        return &it->second;
    }

    CVulkanTexturesCache::~CVulkanTexturesCache() {
        logging::debug("Cleaned up textures cache");
    }

    void CVulkanTexturesCache::transitTextures(const core::CVulkanCommandPool& commandPool, const core::CVulkanQueue& queue) {
        static std::uint64_t counter = 0;

        logging::debug("Transiting textures cache...");
        if (const uint32_t numTextures = this->_indexedMap.map().size(); numTextures < renderConfig::MAX_TEXTURES_IN_CACHE) {
            for (int i = 0; i < renderConfig::MAX_TEXTURES_IN_CACHE - numTextures; i++) {
                this->loadTexture(std::format("default_texture_{}", std::to_string(counter++)), utility::_ERROR_TEXTURE, VK_FORMAT_R8G8B8A8_SRGB);
            }
        }
        const core::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        commandBuffer.beginRecording();
        for (auto& v : this->_indexedMap.map() | std::views::values) {
            v.recordTextureTransition(commandBuffer);
        }
        commandBuffer.endRecording();
        commandBuffer.submitAndWait(queue);
        for (auto& v : this->_indexedMap.map() | std::views::values) {
            v.destroyStgBuffer();
        }
        logging::debug("Transition complete");
    }

    const asset::CVulkanTexture2D& CVulkanTexturesCache::loadTexture(const std::string& id, const asset::CVulkanImageSrc& srcImage, const VkFormat& format) {
        if (this->_indexedMap.map().size() >= renderConfig::MAX_TEXTURES_IN_CACHE) {
            throw std::runtime_error("Texture cache is full");
        }
        if (const auto it = this->_indexedMap.map().find(id); it != this->_indexedMap.map().end()) {
            return it->second;
        }
        auto texture = asset::CVulkanTexture2D {this->_context, id, srcImage, format};
        this->_indexedMap.put(std::move(id), std::move(texture));
        return this->_indexedMap.getValueAtIndex(this->_indexedMap.map().size() - 1);
    }

    const asset::CVulkanTexture2D* CVulkanTexturesCache::loadTexture(const std::string& id, const std::filesystem::path& texturePath, const VkFormat& format) {
        const asset::CVulkanImageSrc srcImage = asset::imageLoad::loadImage(texturePath);
        const asset::CVulkanTexture2D& texture = this->loadTexture(std::move(id), srcImage, format);
        return &texture;
    }

    [[nodiscard]] int CVulkanTexturesCache::getPosition(const std::string* id) const {
        if (id == nullptr) {
            return -1;
        }
        return this->_indexedMap.getIndexOf(*id);
    }


}
