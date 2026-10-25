//
// Created by ded on 08.10.2026.
//

#include "asset_textures.h"
#include <stb_image.hpp>
#include "client/render/vulkan_images.h"
#include "client/render/vulkan_buffers.h"
#include "client/render/vulkan_commands.h"

namespace cvulkan::client::render::asset {
    CVulkanImageSrc::~CVulkanImageSrc() {
        if (this->_data != nullptr) {
            logging::trace("Freeing stbi image data: addr={:p}, size={}", static_cast<void*>(_data), _dataSize);
            stbi_image_free(this->_data);
            this->_data = nullptr;
        }
    }

    CVulkanImageSrc imageLoad::loadImage(const std::filesystem::path& imagePath) {
        int x;
        int y;
        int channels;
        unsigned char* bytes = stbi_load(imagePath.string().c_str(), &x, &y, &channels, 4);
        if (bytes == nullptr) {
            throw std::runtime_error(std::format("Failed to load image '{}'", imagePath.string()));
        }
        return {
            bytes,
            static_cast<size_t>(x) * static_cast<size_t>(y) * 4,
            {x, y},
            static_cast<uint32_t>(channels)
        };
    }

    CVulkanTexture2D::CVulkanTexture2D(const vk::CVulkanContext& context, const std::string& id, const CVulkanImageSrc& imageSrc, const VkFormat& format) : _id(id), _size(imageSrc._size) {
        assert(imageSrc._data != nullptr);
        assert(imageSrc._dataSize > 0);

        logging::debug("Creating texture '{}': data={:p}, size={}", id, static_cast<const void*>(imageSrc._data), imageSrc._dataSize);
        if (imageSrc._data == nullptr || imageSrc._dataSize <= 0) {
            throw std::runtime_error(std::format("Invalid image data for texture '{}'", id));
        }

        this->createStgBuffer(context, imageSrc._data, imageSrc._dataSize);
        const vk::CVulkanImage2DData image2DData {
            ._format = format,
            ._usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            ._extent = {
                .width = static_cast<uint32_t>(imageSrc._size.x),
                .height = static_cast<uint32_t>(imageSrc._size.y),
                .depth = 1
            },
        };
        this->_image = std::make_unique<vk::CVulkanImage2D>(context, image2DData);
        const vk::CVulkanImage2DViewData imageViewData {
            ._aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            ._format = format
        };
        this->_imageView = std::make_unique<vk::CVulkanImage2DView>(context, imageViewData, this->_image->vkImage(), false);
    }

    CVulkanTexture2D::~CVulkanTexture2D() = default;

    CVulkanTexture2D::CVulkanTexture2D(CVulkanTexture2D&& other) noexcept
        : _id(std::move(other._id)),
          _size(other._size),
          _recordedTransition(other._recordedTransition),
          _image(std::move(other._image)),
          _imageView(std::move(other._imageView)),
          _stgBuffer(std::move(other._stgBuffer)) {
    }

    CVulkanTexture2D& CVulkanTexture2D::operator=(CVulkanTexture2D&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        this->_id = std::move(other._id);
        this->_size = other._size;
        this->_recordedTransition = other._recordedTransition;
        this->_image = std::move(other._image);
        this->_imageView = std::move(other._imageView);
        this->_stgBuffer = std::move(other._stgBuffer);
        return *this;
    }

    void CVulkanTexture2D::recordTextureTransition(const vk::CVulkanCommandBuffer& commandBuffer) {
        if (this->_stgBuffer != nullptr && !this->recordedTransitionFlag()) {
            this->_recordedTransition = true;
            vk::img_util::imageBarrier(commandBuffer.vkCommandBuffer(), _image->vkImage(),
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_ACCESS_2_NONE,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT);
            this->recordCopyBuffer(commandBuffer, *this->_stgBuffer);
            vk::img_util::imageBarrier(
                commandBuffer.vkCommandBuffer(), _image->vkImage(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT,
                VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT
            );
        }
    }

    void CVulkanTexture2D::recordCopyBuffer(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanBuffer& bufferData) const {
        const VkBufferImageCopy bufferImageCopy {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {static_cast<uint32_t>(this->_size.x), static_cast<uint32_t>(this->_size.y), 1},
        };
        vkCmdCopyBufferToImage(commandBuffer.vkCommandBuffer(), bufferData.vkBuffer(), this->_image->vkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    }

    void CVulkanTexture2D::destroyStgBuffer() {
        if (this->_stgBuffer != nullptr) {
            this->_stgBuffer.reset();
        }
    }

    void CVulkanTexture2D::createStgBuffer(const vk::CVulkanContext &context, const unsigned char* _data, const size_t& _dataSize) {
        assert(_data != nullptr);
        assert(_dataSize > 0);

        if (_data == nullptr || _dataSize == 0) {
            logging::error("Texture '{}' has invalid image data", this->_id);
            throw std::runtime_error(std::format("Texture '{}' has invalid image data", this->_id));
        }

        logging::debug("Creating staging buffer for texture '{}': {} bytes", this->_id, _dataSize);

        this->_stgBuffer = std::make_unique<vk::CVulkanBuffer>(context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, static_cast<VkDeviceSize>(_dataSize));
        this->_stgBuffer->mapMem();

        void* mapped = this->_stgBuffer->mappedMemory();

        assert(mapped != nullptr);
        if (mapped == nullptr) {
            logging::error("Failed to map staging buffer memory for texture '{}'", this->_id);
            throw std::runtime_error(std::format("Failed to map staging buffer memory for texture '{}'", this->_id));
        }

        std::memcpy(mapped, _data, _dataSize);
        logging::trace("Copied {} bytes to staging buffer for texture '{}'", _dataSize, this->_id);
        this->_stgBuffer->unMapMem();
    }
}
