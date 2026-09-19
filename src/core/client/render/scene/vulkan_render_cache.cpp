//
// Created by ded on 14.09.2026.
//

#include "vulkan_render_cache.h"

namespace cvulkan::client::render::cache {
    std::unique_ptr<CVulkanModelsCache> modelsCache;

    void CVulkanModelsCache::loadModels(const std::vector<structs::CVulkanRawModelData>& models, const core::CVulkanCommandPool& commandPool, const core::CVulkanQueue& queue) {
        std::vector<core::CVulkanBuffer> stagingBuffers{};

        core::CVulkanCommandBuffer commandBuffer{this->_context, commandPool, true, true};
        commandBuffer.createCommandBuffer();
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
        commandBuffer.destroyCommandBuffer();

        for (auto& t: stagingBuffers) {
            t.destroyBuffer();
        }
    }

    void CVulkanModelsCache::destroyCache() {
        for (auto& [id, model]: this->_modelsCache) {
            model.destroyModel();
        }
    }

    const structs::CVulkanModel& CVulkanModelsCache::getModel(const std::string_view id) {
        return this->_modelsCache.at(std::string{id});
    }


    void createCaches(const core::CVulkanContext& context) {
        modelsCache = std::make_unique<CVulkanModelsCache>(context);
    }

    void destroyCaches() {
        modelsCache->destroyCache();
    }
}
