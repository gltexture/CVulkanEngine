//
// Created by ded on 14.09.2026.
//

#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "client/render/vulkan_render_core.h"
#include "client/render/vulkan_buffers.h"

namespace cvulkan::client::renderStructs {
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
        explicit CVulkanVertexStruct(const renderCore::CVulkanContext& context)
            : _context(context) {}
        ~CVulkanVertexStruct() = default;

        CVulkanVertexStruct(const CVulkanVertexStruct&) = delete;
        CVulkanVertexStruct& operator=(const CVulkanVertexStruct&) = delete;

        void initVertexStruct();
        void destroyVertexStruct();

        [[nodiscard]] VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo() const {
            return _vkPipelineVertexInputStateCreateInfo;
        }

    private:
        const renderCore::CVulkanContext& _context;
        VkPipelineVertexInputStateCreateInfo _vkPipelineVertexInputStateCreateInfo {};
        std::vector<VkVertexInputAttributeDescription> _vkVertexInputAttributeDescriptions {};
        std::vector<VkVertexInputBindingDescription> _vkVertexInputBindingDescriptions {};
    };

    class CVulkanMesh {
    public:
        CVulkanMesh(const std::string& id, renderCore::CVulkanBuffer&& verticesBuffer, renderCore::CVulkanBuffer&& indicesBuffer, const uint32_t& numIndices)
            : _id(id),
              _verticesBuffer(std::move(verticesBuffer)),
              _indicesBuffer(std::move(indicesBuffer)),
              _numIndices(numIndices) {
        }

        CVulkanMesh(const CVulkanMesh&) = delete;
        CVulkanMesh& operator=(const CVulkanMesh&) = delete;

        CVulkanMesh(CVulkanMesh&&) noexcept = default;
        CVulkanMesh& operator=(CVulkanMesh&&) = delete;

        void destroyMesh();

        [[nodiscard]] const std::string& id() const {
            return _id;
        }

        [[nodiscard]] const renderCore::CVulkanBuffer& verticesBuffer() const {
            return _verticesBuffer;
        }

        [[nodiscard]] const renderCore::CVulkanBuffer& indicesBuffer() const {
            return _indicesBuffer;
        }

        [[nodiscard]] const uint32_t& numIndices() const {
            return _numIndices;
        }

    private:
        std::string _id;
        renderCore::CVulkanBuffer _verticesBuffer;
        renderCore::CVulkanBuffer _indicesBuffer;
        uint32_t _numIndices;
    };

    class CVulkanModel {
    public:
        explicit CVulkanModel(const std::string& id)
            : _id(id) {}
        ~CVulkanModel() = default;

        CVulkanModel(const CVulkanModel&) = delete;
        CVulkanModel& operator=(const CVulkanModel&) = delete;

        CVulkanModel(CVulkanModel&&) noexcept = default;
        CVulkanModel& operator=(CVulkanModel&&) = delete;

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
