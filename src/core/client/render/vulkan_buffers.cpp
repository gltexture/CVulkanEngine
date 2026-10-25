//
// Created by ded on 13.09.2026.
//

#include "vulkan_buffers.h"

#include "vulkan_commands.h"
#include "assets/asset_models.h"
#include "scene/vulkan_render_structs.h"

namespace cvulkan::client::render::vk {
    CVulkanBuffer::CVulkanBuffer(const CVulkanContext& context, const VkBufferUsageFlags& usage, const uint32_t& reqMask, const VkDeviceSize& size) : _context{context} {
        this->_vkRequestedSize = size;
        const VkBufferCreateInfo bufferCreateInfo  {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = size,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        vk::util::vkCheck(vkCreateBuffer(this->_context.device().vkDevice(), &bufferCreateInfo, nullptr, &this->_vkBuffer), "Failed to create buffer");

        VkMemoryRequirements memoryRequirements  {};
        vkGetBufferMemoryRequirements(this->_context.device().vkDevice(), this->_vkBuffer, &memoryRequirements);

        const VkMemoryAllocateInfo memoryAllocateInfo  {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = vk::util::getMemoryTypeIndexFromProperties(context.physicalDevice().vkMemoryProperties(), memoryRequirements.memoryTypeBits, reqMask),
        };
        this->_vkAllocationSize = memoryAllocateInfo.allocationSize;

        vk::util::vkCheck(vkAllocateMemory(this->_context.device().vkDevice(), &memoryAllocateInfo, nullptr, &this->_vkDeviceMemory), "Failed to allocate memory");
        vk::util::vkCheck(vkBindBufferMemory(this->_context.device().vkDevice(), this->_vkBuffer, this->_vkDeviceMemory, 0), "Failed to bind buffer memory");
    }

    CVulkanBuffer::~CVulkanBuffer() {
        if (this->_vkBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(this->_context.device().vkDevice(), this->_vkBuffer, nullptr);
            this->_vkBuffer = VK_NULL_HANDLE;
        }
        if (this->_vkDeviceMemory != VK_NULL_HANDLE) {
            vkFreeMemory(this->_context.device().vkDevice(), this->_vkDeviceMemory, nullptr);
            this->_vkDeviceMemory = VK_NULL_HANDLE;
        }
    }

    void CVulkanBuffer::mapMem() {
        if (this->_mappedMemory == nullptr) {
            vk::util::vkCheck(vkMapMemory(this->_context.device().vkDevice(), this->_vkDeviceMemory, 0, this->_vkAllocationSize, 0, &this->_mappedMemory), "Failed to bind buffer memory");
        }
    }

    void CVulkanBuffer::unMapMem() {
        if (this->_mappedMemory != nullptr) {
            vkUnmapMemory(this->_context.device().vkDevice(), this->_vkDeviceMemory);
            this->_mappedMemory = nullptr;
        }
    }

    void CVulkanTransferBufferData::recordTransferCommand(const CVulkanCommandBuffer& commandBuffer) const {
        const VkBufferCopy copyRegion  {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = this->_src.vkRequestedSize()
        };
        vkCmdCopyBuffer(commandBuffer.vkCommandBuffer(), this->_src.vkBuffer(), this->_dst.vkBuffer(), 1, &copyRegion);
    }

    CVulkanTransferBufferData createVerticesBuffer(const CVulkanContext& context, const asset::modelGen::CVulkanMeshData& rawMeshData, const std::filesystem::path& vtxPath) {
        const VkDeviceSize bufferSize = rawMeshData._vtxSize;
        logging::debug("Creating vertex transfer buffers from '{}': size={} bytes", vtxPath.string(), bufferSize);

        if (bufferSize == 0) {
            throw std::runtime_error(std::format("Vertex buffer size is zero for file '{}'", vtxPath.string()));
        }
        if (bufferSize > static_cast<VkDeviceSize>(std::numeric_limits<std::streamsize>::max())) {
            throw std::runtime_error(std::format("Vertex buffer size exceeds stream read limit for file '{}': size={}", vtxPath.string(), bufferSize));
        }
        if (!std::filesystem::is_regular_file(vtxPath)) {
            throw std::runtime_error(std::format("Vertex file does not exist or is not a regular file: '{}'", vtxPath.string()));
        }

        const std::uintmax_t fileSize = std::filesystem::file_size(vtxPath);
        if (fileSize != bufferSize) {
            throw std::runtime_error(std::format("Vertex file size mismatch for '{}': expected {} bytes, actual {} bytes", vtxPath.string(), bufferSize, fileSize));
        }

        CVulkanBuffer src{context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize};
        CVulkanBuffer dst{context, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize};

        std::ifstream binFile{vtxPath, std::ios::binary | std::ios::in};
        if (!binFile) {
            throw std::runtime_error(std::format("Failed to open vertex file '{}'", vtxPath.string()));
        }

        logging::debug("Mapping vertex staging buffer: size={} bytes", bufferSize);
        src.mapMem();
        void* mappedMemory = src.mappedMemory();
        if (mappedMemory == nullptr) {
            src.unMapMem();
            throw std::runtime_error(std::format("Failed to map vertex staging buffer for file '{}'", vtxPath.string()));
        }

        binFile.read(static_cast<char*>(mappedMemory), static_cast<std::streamsize>(bufferSize));
        const std::streamsize bytesRead = binFile.gcount();
        src.unMapMem();

        if (bytesRead != static_cast<std::streamsize>(bufferSize)) {
            throw std::runtime_error(std::format("Failed to read vertex file '{}': expected {} bytes, read {} bytes", vtxPath.string(), bufferSize, bytesRead));
        }

        logging::debug("Vertex staging buffer loaded successfully: file='{}', size={} bytes", vtxPath.string(), bufferSize);
        return {std::move(src), std::move(dst)};
    }

    CVulkanTransferBufferData createIndicesBuffers(const CVulkanContext& context, const asset::modelGen::CVulkanMeshData& rawMeshData, const std::filesystem::path& idxPath) {
        const VkDeviceSize bufferSize = rawMeshData._idxSize;
        logging::debug("Creating index transfer buffers from '{}': size={} bytes", idxPath.string(), bufferSize);

        if (bufferSize == 0) {
            throw std::runtime_error(std::format("Index buffer size is zero for file '{}'", idxPath.string()));
        }
        if (bufferSize > static_cast<VkDeviceSize>(std::numeric_limits<std::streamsize>::max())) {
            throw std::runtime_error(std::format("Index buffer size exceeds stream read limit for file '{}': size={}", idxPath.string(), bufferSize));
        }
        if (!std::filesystem::is_regular_file(idxPath)) {
            throw std::runtime_error(std::format("Index file does not exist or is not a regular file: '{}'", idxPath.string()));
        }

        const std::uintmax_t fileSize = std::filesystem::file_size(idxPath);
        if (fileSize != bufferSize) {
            throw std::runtime_error(std::format("Index file size mismatch for '{}': expected {} bytes, actual {} bytes", idxPath.string(), bufferSize, fileSize));
        }

        CVulkanBuffer src{context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize};
        CVulkanBuffer dst{context, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize};

        std::ifstream binFile{idxPath, std::ios::binary | std::ios::in};
        if (!binFile) {
            throw std::runtime_error(std::format("Failed to open index file '{}'", idxPath.string()));
        }

        logging::debug("Mapping index staging buffer: size={} bytes", bufferSize);
        src.mapMem();
        void* mappedMemory = src.mappedMemory();
        if (mappedMemory == nullptr) {
            src.unMapMem();
            throw std::runtime_error(std::format("Failed to map index staging buffer for file '{}'", idxPath.string()));
        }

        binFile.read(static_cast<char*>(mappedMemory), static_cast<std::streamsize>(bufferSize));
        const std::streamsize bytesRead = binFile.gcount();
        src.unMapMem();

        if (bytesRead != static_cast<std::streamsize>(bufferSize)) {
            throw std::runtime_error(std::format("Failed to read index file '{}': expected {} bytes, read {} bytes", idxPath.string(), bufferSize, bytesRead));
        }

        logging::debug("Index staging buffer loaded successfully: file='{}', size={} bytes", idxPath.string(), bufferSize);
        return {std::move(src), std::move(dst)};
    }

    CVulkanBuffer buffutil::allocateHostVisibleBuffer(const CVulkanContext& context, CVulkanDescriptorAllocator& allocator, const VkDeviceSize size, const VkBufferUsageFlags usage, const std::string& id, const CVulkanDescriptorSetLayout& layout) {
        CVulkanBuffer buffer {context, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, size};
        const auto& t = allocator.allocateDescriptorSets(id, 1, layout).front();
        t.setBuffer(buffer, buffer.vkRequestedSize(), layout.layoutInfoArray().front()._binding, layout.layoutInfoArray().front()._descType);
        return buffer;
    }

    void buffutil::copyMatrixToBuffer(CVulkanBuffer& buffer, const glm::mat4& matrix, const uint32_t offset) {
        buffer.mapMem();
        std::memcpy(static_cast<char*>(buffer.mappedMemory()) + offset, &matrix, sizeof(glm::mat4));
        buffer.unMapMem();
    }
}
