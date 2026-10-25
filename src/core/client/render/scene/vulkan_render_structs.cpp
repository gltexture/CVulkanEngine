//
// Created by ded on 14.09.2026.
//

#include "vulkan_render_structs.h"

namespace cvulkan::client::render::structs {
    CVulkanVertexStruct::CVulkanVertexStruct(const vk::CVulkanContext& context) : _context{context} {
    static_assert(NUMBER_OF_ATTRIBUTES == 2);
    static_assert(POSITION_COMPONENTS == 3);
    static_assert(UV_COMPONENTS == 2);

    logging::debug("Creating Vulkan vertex input structure: attributes={}, bindings={}", NUMBER_OF_ATTRIBUTES, 1);

    _vkVertexInputAttributeDescriptions.resize(NUMBER_OF_ATTRIBUTES);
    _vkVertexInputBindingDescriptions.resize(1);

    uint32_t idx = 0;
    uint32_t mOffset = 0;

    {
        auto& [location, binding, format, offset] = _vkVertexInputAttributeDescriptions[idx];
        binding = 0;
        location = idx;
        format = VK_FORMAT_R32G32B32_SFLOAT;
        offset = mOffset;
        mOffset += POSITION_COMPONENTS * sizeof(float);
        logging::debug("Vertex attribute: location={}, binding={}, format=VK_FORMAT_R32G32B32_SFLOAT, offset={}, size={}", location, binding, offset, POSITION_COMPONENTS * sizeof(float));
    }

    {
        idx = 1;
        auto& [location, binding, format, offset] = _vkVertexInputAttributeDescriptions[idx];
        binding = 0;
        location = idx;
        format = VK_FORMAT_R32G32_SFLOAT;
        offset = mOffset;
        mOffset += UV_COMPONENTS * sizeof(float);
        logging::debug("Vertex attribute: location={}, binding={}, format=VK_FORMAT_R32G32_SFLOAT, offset={}, size={}", location, binding, offset, UV_COMPONENTS * sizeof(float));
    }

    {
        const uint32_t m_stride = mOffset;
        auto& [binding, stride, inputRate] = _vkVertexInputBindingDescriptions[0];
        binding = 0;
        stride = m_stride;
        inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        logging::debug("Vertex binding: binding={}, stride={}, inputRate=VK_VERTEX_INPUT_RATE_VERTEX", binding, stride);
    }

    {
        if (_vkVertexInputAttributeDescriptions.size() != NUMBER_OF_ATTRIBUTES || _vkVertexInputBindingDescriptions.size() != 1) {
            throw std::runtime_error("Invalid Vulkan vertex input description counts");
        }

        if (mOffset == 0) {
            throw std::runtime_error("Vulkan vertex stride is zero");
        }

        _vkPipelineVertexInputStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(_vkVertexInputBindingDescriptions.size()),
            .pVertexBindingDescriptions = _vkVertexInputBindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(_vkVertexInputAttributeDescriptions.size()),
            .pVertexAttributeDescriptions = _vkVertexInputAttributeDescriptions.data(),
        };

        logging::debug("Vulkan vertex input structure created: attributes={}, bindings={}, stride={} bytes", _vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount, _vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount, _vkVertexInputBindingDescriptions[0].stride);
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
