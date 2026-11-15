//
// Created by ded on 14.09.2026.
//

#include "vulkan_render_structs.h"

#include <utility>

#include "utility/DEBUG.h"
#include "utility/exceptions.h"

namespace cvulkan::client::render::structs {
    CVulkanMeshVertexBufferStruct::CVulkanMeshVertexBufferStruct(const vk::CVulkanContext& context) : CVulkanVertexBufferStruct(context) {
        static_assert(NUMBER_OF_ATTRIBUTES == 2);
        static_assert(POSITION_COMPONENTS == 3);
        static_assert(UV_COMPONENTS == 2);

        logging::debug("Creating Vulkan Mesh-vertex input structure: attributes={}, bindings={}", NUMBER_OF_ATTRIBUTES, 1);

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
            mOffset += POSITION_COMPONENTS * sizeof(float);
            logging::debug("Mesh-Vertex attribute: location={}, binding={}, format=VK_FORMAT_R32G32B32_SFLOAT, offset={}, size={}", location, binding, offset, POSITION_COMPONENTS * sizeof(float));
        }

        {
            idx = 1;
            auto& [location, binding, format, offset] = this->_vkVertexInputAttributeDescriptions[idx];
            binding = 0;
            location = idx;
            format = VK_FORMAT_R32G32_SFLOAT;
            offset = mOffset;
            mOffset += UV_COMPONENTS * sizeof(float);
            logging::debug("Mesh-Vertex attribute: location={}, binding={}, format=VK_FORMAT_R32G32_SFLOAT, offset={}, size={}", location, binding, static_cast<uint32_t>(format), offset, UV_COMPONENTS * sizeof(float));
        }

        {
            const uint32_t m_stride = mOffset;
            auto& [binding, stride, inputRate] = this->_vkVertexInputBindingDescriptions[0];
            binding = 0;
            stride = m_stride;
            inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            logging::debug("Mesh-Vertex binding: binding={}, stride={}, inputRate=VK_VERTEX_INPUT_RATE_VERTEX", binding, stride);
        }

        DEBUG::ASSERT(this->_vkVertexInputAttributeDescriptions.size() == NUMBER_OF_ATTRIBUTES && this->_vkVertexInputBindingDescriptions.size() == 1, "Invalid Vulkan Mesh-vertex input description counts");
        DEBUG::ASSERT(mOffset > 0, "Vulkan Mesh-vertex stride is zero");

        this->_vkPipelineVertexInputStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(this->_vkVertexInputBindingDescriptions.size()),
            .pVertexBindingDescriptions = this->_vkVertexInputBindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(this->_vkVertexInputAttributeDescriptions.size()),
            .pVertexAttributeDescriptions = this->_vkVertexInputAttributeDescriptions.data(),
        };

        logging::debug("Vulkan Mesh-vertex input structure created: attributes={}, bindings={}, stride={} bytes", this->_vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount, this->_vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount, this->_vkVertexInputBindingDescriptions[0].stride);
    }


    CVulkanImGuiVertexBufferStruct::CVulkanImGuiVertexBufferStruct(const vk::CVulkanContext& context) : CVulkanVertexBufferStruct(context) {
        static_assert(NUMBER_OF_ATTRIBUTES == 3);

        logging::debug("Creating Vulkan ImGui-vertex input structure: attributes={}, bindings={}", NUMBER_OF_ATTRIBUTES, 1);

        this->_vkVertexInputAttributeDescriptions.resize(NUMBER_OF_ATTRIBUTES);
        this->_vkVertexInputBindingDescriptions.resize(1);

        uint32_t idx = 0;
        uint32_t mOffset = 0;

        {   // POS
            auto& [location, binding, format, offset] = this->_vkVertexInputAttributeDescriptions[idx];
            binding = 0;
            location = idx;
            format = VK_FORMAT_R32G32_SFLOAT;
            offset = mOffset;
            mOffset += 2 * sizeof(float);
            logging::debug("ImGui-Vertex attribute: location={}, binding={}, format=VK_FORMAT_R32G32_SFLOAT, offset={}, size={}", location, binding, offset, 2 * sizeof(float));
        }

        {   // UV
            idx = 1;
            auto& [location, binding, format, offset] = this->_vkVertexInputAttributeDescriptions[idx];
            binding = 0;
            location = idx;
            format = VK_FORMAT_R32G32_SFLOAT;
            offset = mOffset;
            mOffset += 2 * sizeof(float);
            logging::debug("ImGui-Vertex attribute: location={}, binding={}, format=VK_FORMAT_R32G32_SFLOAT, offset={}, size={}", location, binding, offset, 2 * sizeof(float));
        }

        {   // COLOR
            idx = 2;
            auto& [location, binding, format, offset] = this->_vkVertexInputAttributeDescriptions[idx];
            binding = 0;
            location = idx;
            format = VK_FORMAT_R8G8B8A8_UNORM;
            offset = mOffset;
            mOffset += sizeof(uint8_t) * 4;
            logging::debug("ImGui-Vertex attribute: location={}, binding={}, format=VK_FORMAT_R8G8B8A8_UNORM, offset={}, size={}", location, binding, offset, sizeof(uint8_t) * 4);
        }

        {
            auto& [binding, stride, inputRate] = this->_vkVertexInputBindingDescriptions[0];
            binding = 0;
            stride = VERTEX_SIZE;
            inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            logging::debug("ImGui-Vertex binding: binding={}, stride={}, inputRate=VK_VERTEX_INPUT_RATE_VERTEX", binding, stride);
        }

        DEBUG::ASSERT(this->_vkVertexInputAttributeDescriptions.size() == NUMBER_OF_ATTRIBUTES && this->_vkVertexInputBindingDescriptions.size() == 1, "Invalid ImGui Mesh-vertex input description counts");
        DEBUG::ASSERT(mOffset > 0, "Vulkan ImGui-vertex stride is zero");
        DEBUG::ASSERT(mOffset == VERTEX_SIZE, "ImGui vertex stride mismatch: calculated={}, expected={}", mOffset, VERTEX_SIZE);

        this->_vkPipelineVertexInputStateCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(this->_vkVertexInputBindingDescriptions.size()),
            .pVertexBindingDescriptions = this->_vkVertexInputBindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(this->_vkVertexInputAttributeDescriptions.size()),
            .pVertexAttributeDescriptions = this->_vkVertexInputAttributeDescriptions.data(),
        };

        logging::debug("Vulkan ImGui-vertex input structure created: attributes={}, bindings={}, stride={} bytes",
            this->_vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount, this->_vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount, this->_vkVertexInputBindingDescriptions[0].stride);
    }

    CVulkanVertexBufferStruct::~CVulkanVertexBufferStruct() = default;

    CVulkanMeshVertexBufferStruct::~CVulkanMeshVertexBufferStruct() = default;

    CVulkanImGuiVertexBufferStruct::~CVulkanImGuiVertexBufferStruct() = default;

    CVulkanMesh::CVulkanMesh(std::string id, vk::CVulkanBuffer&& verticesBuffer, vk::CVulkanBuffer&& indicesBuffer, const uint32_t& numIndices, std::string materialId)
        : _id(std::move(id)), _verticesBuffer(std::move(verticesBuffer)), _indicesBuffer(std::move(indicesBuffer)), _numIndices(numIndices), _materialId(std::move(materialId)) {
    }

    CVulkanMesh::~CVulkanMesh() = default;

    CVulkanModel::CVulkanModel(std::string  id) : _id(std::move(id)) {
    }

    CVulkanModel::~CVulkanModel() = default;
}
