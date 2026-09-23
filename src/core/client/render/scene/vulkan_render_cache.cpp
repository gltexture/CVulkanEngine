//
// Created by ded on 14.09.2026.
//

#include "vulkan_render_cache.h"

namespace cvulkan::client::render::cache {
    CVulkanModelsCache::~CVulkanModelsCache() {
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

    const structs::CVulkanModel& CVulkanModelsCache::getModel(const std::string_view id) {
        return this->_modelsCache.at(std::string{id});
    }
}
