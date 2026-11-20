//
// Created by ded on 14.09.2026.
//

#pragma once

#include <string>
#include <vector>
#include "client/render/vulkan_render_core.h"
#include "client/render/vulkan_buffers.h"

namespace cvulkan::client::render::structs {
    //struct CVulkanRawMeshData {
    //    std::string id;
    //    std::vector<float> positions;
    //    std::vector<float> uv;
    //    std::vector<uint32_t> indices;
    //};

    struct CVulkanMaterial {
        std::string _id;
        bool _hasTransparency;
    };

    class CVulkanVertexBufferStruct {
    public:
        explicit CVulkanVertexBufferStruct(const vk::CVulkanContext& context) : _context{context} {
        }
        ~CVulkanVertexBufferStruct();

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanVertexBufferStruct);
        CVulkanVertexBufferStruct(CVulkanVertexBufferStruct&& other) noexcept :
        _context{other._context},
        _vkPipelineVertexInputStateCreateInfo{other._vkPipelineVertexInputStateCreateInfo},
        _vkVertexInputAttributeDescriptions{std::move(other._vkVertexInputAttributeDescriptions)},
        _vkVertexInputBindingDescriptions{std::move(other._vkVertexInputBindingDescriptions)} {
            _vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = _vkVertexInputAttributeDescriptions.data();
            _vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions =_vkVertexInputBindingDescriptions.data();
        }

        [[nodiscard]] const VkPipelineVertexInputStateCreateInfo& vkPipelineVertexInputStateCreateInfo() const {
            return _vkPipelineVertexInputStateCreateInfo;
        }

    protected:
        const vk::CVulkanContext& _context;
        VkPipelineVertexInputStateCreateInfo _vkPipelineVertexInputStateCreateInfo{};
        std::vector<VkVertexInputAttributeDescription> _vkVertexInputAttributeDescriptions{};
        std::vector<VkVertexInputBindingDescription> _vkVertexInputBindingDescriptions{};
    };

    class CVulkanMeshVertexBufferStruct : public CVulkanVertexBufferStruct {
    public:
        static constexpr uint32_t NUMBER_OF_ATTRIBUTES = 2;
        static constexpr uint32_t POSITION_COMPONENTS = 3;
        static constexpr uint32_t UV_COMPONENTS = 2;

        explicit CVulkanMeshVertexBufferStruct(const vk::CVulkanContext& context);
        ~CVulkanMeshVertexBufferStruct();

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanMeshVertexBufferStruct);
        CVulkanMeshVertexBufferStruct(CVulkanMeshVertexBufferStruct&& other) noexcept = default;
    };

    class CVulkanImGuiVertexBufferStruct : public CVulkanVertexBufferStruct {
    public:
        static constexpr uint32_t NUMBER_OF_ATTRIBUTES = 3;
        static constexpr uint32_t VERTEX_SIZE = sizeof(float) * 5;

        explicit CVulkanImGuiVertexBufferStruct(const vk::CVulkanContext& context);
        ~CVulkanImGuiVertexBufferStruct();

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanImGuiVertexBufferStruct);
        CVulkanImGuiVertexBufferStruct(CVulkanImGuiVertexBufferStruct&& other) noexcept = default;
    };

    class CVulkanMesh {
    public:
        CVulkanMesh(std::string  id, vk::CVulkanBuffer&& verticesBuffer, vk::CVulkanBuffer&& indicesBuffer, const uint32_t& numIndices, std::string  materialId);

        ~CVulkanMesh();

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanMesh);

        CVulkanMesh(CVulkanMesh&& other) noexcept = default;

        [[nodiscard]] const std::string& id() const {
            return _id;
        }

        [[nodiscard]] const vk::CVulkanBuffer& verticesBuffer() const {
            return _verticesBuffer;
        }

        [[nodiscard]] const vk::CVulkanBuffer& indicesBuffer() const {
            return _indicesBuffer;
        }

        [[nodiscard]] uint32_t numIndices() const {
            return _numIndices;
        }

        [[nodiscard]] const std::string& materialId() const {
            return _materialId;
        }

    private:
        std::string _id;
        vk::CVulkanBuffer _verticesBuffer;
        vk::CVulkanBuffer _indicesBuffer;
        uint32_t _numIndices;
        std::string _materialId;
    };

    class CVulkanModel {
    public:
        explicit CVulkanModel(std::string  id);

        ~CVulkanModel();

        CLASS_NO_COPY_NO_MOVEASSIGN(CVulkanModel);

        CVulkanModel(CVulkanModel&& other) noexcept = default;

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
        std::vector<CVulkanMesh> _meshes{};
    };
}
