//
// Created by ded on 14.09.2026.
//

#include "vulkan_render_structs.h"

namespace cvulkan::client::render::structs {
    CVulkanVertexStruct::CVulkanVertexStruct(const vk::CVulkanContext& context) : _context(context) {
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
            idx = 1;
            mOffset += POSITION_COMPONENTS * sizeof(float);
            auto& [location, binding, format, offset] = this->_vkVertexInputAttributeDescriptions[idx];
            binding = 0;
            location = idx;
            format = VK_FORMAT_R32G32_SFLOAT;
            offset = mOffset;
        }

        {
            const uint32_t m_stride = mOffset + UV_COMPONENTS * sizeof(float);
            auto& [binding, stride, inputRate] = this->_vkVertexInputBindingDescriptions[0];
            binding = 0;
            stride = m_stride;
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

    CVulkanVertexStruct::~CVulkanVertexStruct() {
    }

    CVulkanMesh::CVulkanMesh(const std::string& id, vk::CVulkanBuffer&& verticesBuffer, vk::CVulkanBuffer&& indicesBuffer, const uint32_t& numIndices, const std::string& materialId)
        : _id(id), _verticesBuffer(std::move(verticesBuffer)), _indicesBuffer(std::move(indicesBuffer)), _numIndices(numIndices), _materialId(materialId) {
    }

    CVulkanMesh::~CVulkanMesh() {
    }

    CVulkanModel::CVulkanModel(const std::string& id) : _id(id) {
    }

    CVulkanModel::~CVulkanModel() {
    }
}
