//
// Created by ded on 14.09.2026.
//

#include "vulkan_render_structs.h"

namespace cvulkan::client::render::structs {
    void CVulkanVertexStruct::createVertexStruct() {
        this->_vkVertexInputAttributeDescriptions.resize(NUMBER_OF_ATTRIBUTES);
        this->_vkVertexInputBindingDescriptions.resize(1);

        uint32_t idx = 0;
        uint32_t mOffset = 0;

        {
            auto& [location, binding, format, offset] = this->_vkVertexInputAttributeDescriptions[idx];
            binding = 0;
            location = idx;
            format = VK_FORMAT_R32G32B32_SFLOAT;
            offset = mOffset;
        }

        {
            auto& [binding, stride, inputRate] = this->_vkVertexInputBindingDescriptions[0];
            binding = 0;
            stride = POSITION_COMPONENTS * sizeof(float);
            inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        {
            this->_vkPipelineVertexInputStateCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = static_cast<uint32_t>(this->_vkVertexInputBindingDescriptions.size()),
                .pVertexBindingDescriptions = this->_vkVertexInputBindingDescriptions.data(),
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(this->_vkVertexInputAttributeDescriptions.size()),
                .pVertexAttributeDescriptions = this->_vkVertexInputAttributeDescriptions.data(),
            };
        }
    }

    void CVulkanVertexStruct::destroyVertexStruct() {
    }

    void CVulkanMesh::destroyMesh() {
        this->_indicesBuffer.destroyBuffer();
        this->_verticesBuffer.destroyBuffer();
    }

    void CVulkanModel::destroyModel() {
        for (auto& t : this->_meshes) {
            t.destroyMesh();
        }
    }
}
