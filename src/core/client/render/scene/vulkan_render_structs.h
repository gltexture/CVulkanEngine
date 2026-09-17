//
// Created by ded on 14.09.2026.
//

#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "client/render/vulkan_render_core.h"
#include "client/render/vulkan_buffers.h"

namespace cvulkan::client::render::structs {
    static constexpr uint32_t NUMBER_OF_ATTRIBUTES = 1;
    static constexpr uint32_t POSITION_COMPONENTS = 3;


    struct CVulkanRawMeshData {
        std::string id;
        std::vector<float> positions;
        std::vector<uint32_t> indices;
    };

    struct CVulkanRawModelData {
        std::string id;
        std::vector<CVulkanRawMeshData> meshes;
    };


    class CVulkanVertexStruct {
    public:
        explicit CVulkanVertexStruct(const core::CVulkanContext& context)
            : _context(context) {}
        ~CVulkanVertexStruct() = default;

        CVULKAN_NO_COPY(CVulkanVertexStruct);

        void createVertexStruct();
        void destroyVertexStruct();

        [[nodiscard]] VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo() const {
            return _vkPipelineVertexInputStateCreateInfo;
        }

    private:
        const core::CVulkanContext& _context;
        VkPipelineVertexInputStateCreateInfo _vkPipelineVertexInputStateCreateInfo {};
        std::vector<VkVertexInputAttributeDescription> _vkVertexInputAttributeDescriptions {};
        std::vector<VkVertexInputBindingDescription> _vkVertexInputBindingDescriptions {};
    };

    class CVulkanMesh {
    public:
        CVulkanMesh(const std::string& id, core::CVulkanBuffer&& verticesBuffer, core::CVulkanBuffer&& indicesBuffer, const uint32_t& numIndices)
            : _id(id),
              _verticesBuffer(std::move(verticesBuffer)),
              _indicesBuffer(std::move(indicesBuffer)),
              _numIndices(numIndices) {
        }

        CVULKAN_NO_COPY_NO_MOVE(CVulkanMesh);

        void destroyMesh();

        [[nodiscard]] const std::string& id() const {
            return _id;
        }

        [[nodiscard]] const core::CVulkanBuffer& verticesBuffer() const {
            return _verticesBuffer;
        }

        [[nodiscard]] const core::CVulkanBuffer& indicesBuffer() const {
            return _indicesBuffer;
        }

        [[nodiscard]] const uint32_t& numIndices() const {
            return _numIndices;
        }

    private:
        std::string _id;
        core::CVulkanBuffer _verticesBuffer;
        core::CVulkanBuffer _indicesBuffer;
        uint32_t _numIndices;
    };

    class CVulkanModel {
    public:
        explicit CVulkanModel(const std::string& id)
            : _id(id) {}
        ~CVulkanModel() = default;

        CVULKAN_NO_COPY_NO_MOVE(CVulkanModel);

        void destroyModel();

        [[nodiscard]] const std::string& id() const {
            return _id;
        }

        [[nodiscard]] std::vector<CVulkanMesh>& meshes() {
            return _meshes;
        }

        [[nodiscard]] const std::vector<CVulkanMesh>& meshes() const {
            return _meshes;
        }

    private:
        std::string _id;
        std::vector<CVulkanMesh> _meshes {};
    };
}
