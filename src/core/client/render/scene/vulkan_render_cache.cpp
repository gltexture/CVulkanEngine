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
        logging::debug("Loading {} models into Vulkan", models.size());

        if (models.empty()) {
            logging::debug("No models to load");
            return;
        }

        std::vector<vk::CVulkanBuffer> stagingBuffers{}; // LIFECYCLE SAVE

        vk::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        logging::debug("Beginning model transfer command recording");
        commandBuffer.beginRecording();

        uint64_t totalMeshes = 0;
        uint64_t totalStagingBytes = 0;

        for (const auto& [id, meshes, _materialsPath_Relative, _cachePath_Relative, _idxPath_Relative, _vtxPath_Relative] : models) {
            logging::debug("Loading model '{}': meshes={}", id, meshes.size());

            const auto [modelIt, inserted] = this->_modelsMap.emplace(id, structs::CVulkanModel{id});
            if (!inserted) {
                throw std::runtime_error(std::format("Model '{}' is already loaded in Vulkan models cache", id));
            }

            for (const auto& mesh : meshes) {
                logging::debug("Loading mesh '{}' for model '{}': vertexSize={} bytes, indexSize={} bytes, materialId={}", mesh._id, id, mesh._vtxSize, mesh._idxSize, mesh._materialId);

                if (mesh._idxSize == 0 || mesh._idxSize % sizeof(uint32_t) != 0) {
                    throw std::runtime_error(std::format("Invalid index buffer size for mesh '{}' in model '{}': size={} bytes, expected a non-zero multiple of {}", mesh._id, id, mesh._idxSize, sizeof(uint32_t)));
                }

                const auto verticesPath = io_utils::RES_RESOLVER / _vtxPath_Relative;
                const auto indicesPath = io_utils::RES_RESOLVER / _idxPath_Relative;

                vk::CVulkanTransferBufferData verticesBuffers = vk::createVerticesBuffer(this->_context, mesh, verticesPath);
                vk::CVulkanTransferBufferData indicesBuffer = vk::createIndicesBuffers(this->_context, mesh, indicesPath);

                verticesBuffers.recordTransferCommand(commandBuffer);
                indicesBuffer.recordTransferCommand(commandBuffer);

                stagingBuffers.emplace_back(std::move(verticesBuffers._src));
                stagingBuffers.emplace_back(std::move(indicesBuffer._src));

                totalStagingBytes += mesh._vtxSize + mesh._idxSize;
                ++totalMeshes;

                structs::CVulkanMesh vulkanMesh{mesh._id, std::move(verticesBuffers._dst), std::move(indicesBuffer._dst), static_cast<uint32_t>(mesh._idxSize / sizeof(uint32_t)), mesh._materialId};
                modelIt->second.meshes().emplace_back(std::move(vulkanMesh));

                logging::debug("Mesh '{}' loaded into transfer queue: indexCount={}, stagingBuffers={}", mesh._id, mesh._idxSize / sizeof(uint32_t), stagingBuffers.size());
            }
        }

        logging::debug("Finished recording model transfers: models={}, meshes={}, stagingBuffers={}, stagingBytes={}", models.size(), totalMeshes, stagingBuffers.size(), totalStagingBytes);
        commandBuffer.endRecording();

        logging::debug("Submitting model transfer commands and waiting for completion");
        commandBuffer.submitAndWait(queue);

        logging::debug("Model loading completed: models={}, meshes={}, stagingBytes={}", models.size(), totalMeshes, totalStagingBytes);
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

        logging::debug("Transitioning textures cache layouts");

        const auto numTextures = this->_indexedTexturesMap.map().size();
        logging::debug("Texture cache contains {} textures, maximum={}", numTextures, renderConfig::MAX_TEXTURES_IN_CACHE);

        if (numTextures > renderConfig::MAX_TEXTURES_IN_CACHE) {
            throw std::runtime_error(std::format("Texture cache exceeds maximum capacity: current={}, maximum={}", numTextures, renderConfig::MAX_TEXTURES_IN_CACHE));
        }

        if (numTextures < renderConfig::MAX_TEXTURES_IN_CACHE) {
            const auto numDefaultTextures = renderConfig::MAX_TEXTURES_IN_CACHE - numTextures;
            logging::debug("Creating {} default textures", numDefaultTextures);

            for (std::size_t i = 0; i < numDefaultTextures; i++) {
                const std::string id = std::format("default_texture_{}", counter++);
                logging::debug("Creating default texture '{}'", id);
                this->loadTexture(id, io_utils::RES_RESOLVER / io_utils::CVulkanResourcePath{io_utils::ERROR_TEXTURE}, VK_FORMAT_R8G8B8A8_SRGB);
            }
        }

        logging::debug("Recording texture layout transitions for {} textures", this->_indexedTexturesMap.map().size());

        const vk::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        commandBuffer.beginRecording();

        std::size_t transitionCount = 0;
        for (auto& v : this->_indexedTexturesMap.map() | std::views::values) {
            v.recordTextureTransition(commandBuffer);
            ++transitionCount;
        }

        logging::debug("Recorded {} texture layout transitions", transitionCount);
        commandBuffer.endRecording();

        logging::debug("Submitting texture layout transitions and waiting for completion");
        commandBuffer.submitAndWait(queue);

        logging::debug("Destroying staging buffers for {} textures", transitionCount);
        for (auto& v : this->_indexedTexturesMap.map() | std::views::values) {
            v.destroyStgBuffer();
        }

        logging::debug("Transitioned texture cache layouts: textures={}", transitionCount);
    }

    const asset::CVulkanTexture2D& CVulkanTexturesCache::loadTexture(const std::string& id, const asset::CVulkanImageSrc& srcImage, const VkFormat& format) {
        if (const auto it = this->_indexedTexturesMap.map().find(id); it != this->_indexedTexturesMap.map().end()) {
            logging::debug("Texture '{}' already exists in cache, reusing it", id);
            return it->second;
        }

        if (this->_indexedTexturesMap.map().size() >= renderConfig::MAX_TEXTURES_IN_CACHE) {
            throw std::runtime_error(std::format("Texture cache is full: cannot load texture '{}', current={}, maximum={}", id, this->_indexedTexturesMap.map().size(), renderConfig::MAX_TEXTURES_IN_CACHE));
        }

        logging::debug("Loading texture '{}': format={}", id, static_cast<uint32_t>(format));

        auto texture = asset::CVulkanTexture2D{this->_context, id, srcImage, format};
        this->_indexedTexturesMap.emplace(id, std::move(texture));

        const auto it = this->_indexedTexturesMap.map().find(id);
        if (it == this->_indexedTexturesMap.map().end()) {
            throw std::runtime_error(std::format("Texture '{}' was not found in cache after insertion", id));
        }

        logging::debug("Texture '{}' loaded into cache: textures={}", id, this->_indexedTexturesMap.map().size());
        return it->second;
    }

    const asset::CVulkanTexture2D* CVulkanTexturesCache::loadTexture(const std::string& id, const std::filesystem::path& texturePath, const VkFormat& format) {
        if (const auto it = this->_indexedTexturesMap.map().find(id); it != this->_indexedTexturesMap.map().end()) {
            logging::debug("Texture '{}' already exists in cache, reusing it", id);
            return &it->second;
        }

        logging::debug("Loading texture '{}' from '{}': format={}", id, texturePath.string(), static_cast<uint32_t>(format));
        const asset::CVulkanImageSrc srcImage = asset::imageLoad::loadImage(texturePath);
        const asset::CVulkanTexture2D& texture = this->loadTexture(id, srcImage, format);
        return &texture;
    }

    [[nodiscard]] int32_t CVulkanTexturesCache::getPosition(const std::string& id) const {
        const auto res = this->_indexedTexturesMap.getIndexOf(id);
        if (res < 0) {
            logging::debug("Texture '{}' was not found in cache", id);
            return -1;
        }

        logging::trace("Texture '{}' found in cache at position {}", id, res);
        return res;
    }

    CVulkanMaterialsCache::~CVulkanMaterialsCache() {
        logging::debug("Cleaned up materials cache");
    }

    void CVulkanMaterialsCache::loadMaterials(const std::vector<asset::modelGen::CVulkanMaterialData>& materials, CVulkanTexturesCache& texturesCache, const vk::CVulkanCommandPool& commandPool, const vk::CVulkanQueue& queue) {
        const std::size_t numMaterials = materials.size();
        logging::debug("Loading {} materials into Vulkan", numMaterials);

        if (materials.empty()) {
            logging::debug("No materials to load");
            return;
        }

        constexpr VkDeviceSize materialDataSize = sizeof(glm::vec4) + 4 * sizeof(int32_t);
        if (MAT_BUFFER_SIZE != materialDataSize) {
            throw std::runtime_error(std::format("Material buffer size mismatch: MAT_BUFFER_SIZE={}, expected={}", MAT_BUFFER_SIZE, materialDataSize));
        }

        const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(MAT_BUFFER_SIZE) * numMaterials;
        logging::debug("Allocating materials storage buffer: materials={}, bufferSize={} bytes, materialSize={} bytes", numMaterials, bufferSize, MAT_BUFFER_SIZE);

        vk::CVulkanBuffer srcBuffer{this->_context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize};
        vk::CVulkanBuffer matBuffer{this->_context, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize};

        const vk::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        commandBuffer.beginRecording();

        vk::CVulkanTransferBufferData transferBufferData{
            ._src = std::move(srcBuffer),
            ._dst = std::move(matBuffer),
        };

        logging::debug("Mapping materials staging buffer: size={} bytes", bufferSize);
        transferBufferData._src.mapMem();

        auto* mappedMemory = static_cast<char*>(transferBufferData._src.mappedMemory());
        if (mappedMemory == nullptr) {
            transferBufferData._src.unMapMem();
            throw std::runtime_error("Failed to map materials staging buffer");
        }

        VkDeviceSize offset = 0;
        std::size_t texturedMaterials = 0;

        for (std::size_t i = 0; i < numMaterials; i++) {
            const auto& [_id, _texturePath, _diffuseColor] = materials.at(i);
            logging::debug("Processing material '{}': index={}, texturePath='{}'", _id, i, _texturePath.path().string());

            if (offset + MAT_BUFFER_SIZE > bufferSize) {
                transferBufferData._src.unMapMem();
                throw std::runtime_error(std::format("Materials buffer overflow while writing material '{}': offset={}, materialSize={}, bufferSize={}", _id, offset, MAT_BUFFER_SIZE, bufferSize));
            }

            int32_t texturePos = -1;
            int32_t hasTextureValue32 = 0;

            if (! _texturePath.isEmpty()) {
                const std::filesystem::path texturePath = io_utils::RES_RESOLVER / _texturePath;
                if (std::filesystem::is_regular_file(texturePath)) {
                    texturesCache.loadTexture(texturePath.string(), texturePath, VK_FORMAT_R8G8B8A8_SRGB);
                    texturePos = texturesCache.getPosition(texturePath.string());
                    if (texturePos < 0) {
                        transferBufferData._src.unMapMem();
                        throw std::runtime_error(std::format("Texture '{}' was loaded but has no valid cache position for material '{}'", texturePath.string(), _id));
                    }
                    hasTextureValue32 = 1;
                    ++texturedMaterials;
                } else {
                    logging::warn("Texture file does not exist for material '{}': '{}'. Material will use diffuse color only", _id, texturePath.string());
                }
            }

            const structs::CVulkanMaterial material{_id};
            if (this->_indexedMaterialsMap.map().contains(material._id)) {
                logging::warn("Material '{}' already exists in cache and will be replaced", material._id);
            }
            this->_indexedMaterialsMap.emplace(material._id, material);

            std::memcpy(mappedMemory + offset, &_diffuseColor, sizeof(glm::vec4)); offset += sizeof(glm::vec4);
            std::memcpy(mappedMemory + offset, &hasTextureValue32, sizeof(hasTextureValue32)); offset += sizeof(hasTextureValue32);
            std::memcpy(mappedMemory + offset, &texturePos, sizeof(texturePos)); offset += sizeof(texturePos);

            {
                constexpr int32_t padding = 0;
                std::memcpy(mappedMemory + offset, &padding, sizeof(padding)); offset += sizeof(padding);
                std::memcpy(mappedMemory + offset, &padding, sizeof(padding)); offset += sizeof(padding);
            }

            logging::debug("Material '{}' serialized: textureEnabled={}, texturePosition={}, bytesWritten={}", _id, hasTextureValue32, texturePos, offset);
        }

        transferBufferData._src.unMapMem();

        if (offset != bufferSize) {
            throw std::runtime_error(std::format("Materials buffer size mismatch after serialization: expected={} bytes, written={} bytes", bufferSize, offset));
        }

        logging::debug("Materials serialized successfully: materials={}, texturedMaterials={}, bytesWritten={}", numMaterials, texturedMaterials, offset);

        transferBufferData.recordTransferCommand(commandBuffer);
        commandBuffer.endRecording();

        logging::debug("Submitting materials buffer transfer and waiting for completion");
        commandBuffer.submitAndWait(queue);

        this->_materialsBuffer = std::make_unique<vk::CVulkanBuffer>(std::move(transferBufferData._dst));
        logging::debug("Materials storage buffer uploaded successfully: materials={}, size={} bytes", numMaterials, bufferSize);
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
            throw std::runtime_error(std::format("Material with id '{}' not found", id));
        }
        return static_cast<uint32_t>(res);
    }
}
