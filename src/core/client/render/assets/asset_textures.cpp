//
// Created by ded on 08.10.2026.
//

#include "asset_textures.h"

#include <future>
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
        const uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imageSrc._size.x, imageSrc._size.y)))) + 1;
        const vk::CVulkanImage2DData image2DData {
            ._format = format,
            ._mipLevels = mipLevels,
            ._usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            ._extent = {
                .width = static_cast<uint32_t>(imageSrc._size.x),
                .height = static_cast<uint32_t>(imageSrc._size.y),
                .depth = 1
            }
        };
        this->_image = std::make_unique<vk::CVulkanImage2D>(context, image2DData);
        const vk::CVulkanImage2DViewData imageViewData {
            ._aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            ._format = format,
            ._mipLevels = mipLevels
        };
        this->_imageView = std::make_unique<vk::CVulkanImage2DView>(context, imageViewData, this->_image->vkImage(), false);


        this->transparencyCheck(imageSrc);
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
            this->recordMipMapGeneration(commandBuffer);
           //vk::img_util::imageBarrier(
           //    commandBuffer.vkCommandBuffer(), _image->vkImage(),
           //    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
           //    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
           //    VK_PIPELINE_STAGE_2_TRANSFER_BIT,
           //    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
           //    VK_ACCESS_2_TRANSFER_WRITE_BIT,
           //    VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
           //    VK_IMAGE_ASPECT_COLOR_BIT
           //);
        }
    }

    void CVulkanTexture2D::recordMipMapGeneration(const vk::CVulkanCommandBuffer& commandBuffer) const {
        VkImageSubresourceRange imageSubresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };

        VkImageMemoryBarrier2 barrier {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = this->_image->vkImage(),
            .subresourceRange = imageSubresourceRange,
        };

        const VkDependencyInfo dependencyInfo {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        int32_t mipWidth = this->_size.x;
        int32_t mipHeight = this->_size.y;
        const uint32_t mipLevels = this->image()->mipLevels();

        for (uint32_t i = 1; i < mipLevels; i++) {
            imageSubresourceRange.baseMipLevel = i - 1;
            {
                barrier.subresourceRange = imageSubresourceRange;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                barrier.dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                vkCmdPipelineBarrier2(commandBuffer.vkCommandBuffer(), &dependencyInfo);
            }

            const uint32_t aux_i = i;
            constexpr VkOffset3D srcOffset0 {0, 0, 0};
            const VkOffset3D srcOffset1 {mipWidth, mipHeight, 1};
            constexpr VkOffset3D dstOffset0 {0, 0, 0};
            const VkOffset3D dstOffset1 {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};

            const VkImageBlit blitInfo {
                .srcSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = aux_i - 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .srcOffsets = {srcOffset0, srcOffset1},
                .dstSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = aux_i,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                },
                .dstOffsets = {dstOffset0, dstOffset1}
            };
            vkCmdBlitImage(commandBuffer.vkCommandBuffer(), this->_image->vkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, this->_image->vkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitInfo, VK_FILTER_LINEAR);

            {
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
                barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                vkCmdPipelineBarrier2(commandBuffer.vkCommandBuffer(), &dependencyInfo);
            }
            if (mipWidth > 1) {
                mipWidth /= 2;
            }
            if (mipHeight > 1) {
                mipHeight /= 2;
            }
        }

        imageSubresourceRange.baseMipLevel = mipLevels - 1;
        {
            barrier.subresourceRange = imageSubresourceRange;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            vkCmdPipelineBarrier2(commandBuffer.vkCommandBuffer(), &dependencyInfo);
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

    void CVulkanTexture2D::createStgBuffer(const vk::CVulkanContext& context, const unsigned char* _data, const size_t& _dataSize) {
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

    void CVulkanTexture2D::transparencyCheck(const CVulkanImageSrc& imageSrc) {
        if (imageSrc._channels != 4) {
            return;
        }
        if (imageSrc._data[3] < 255) {
            this->_hasTransparency = true;
            return;
        }
        std::atomic<bool> result{false};
        const uint32_t numPixels = imageSrc._size.x * imageSrc._size.y;
        {
            std::vector<std::future<void>> futures {};
            for (uint32_t j = 0; j < 4; j++) {
                const size_t start = numPixels * j / 4;
                const size_t end = numPixels * (j + 1) / 4;
                futures.push_back(std::async(std::launch::async, [&imageSrc, &result, start, end]() {
                   for (size_t i = start; i < end; i++) {
                       if (result) {
                           return;
                       }
                       if (imageSrc._data[i * 4 + 3] < 255) {
                           result.store(true);
                           return;
                       }
                   }
               }));
            }
            for (auto& future : futures) {
                future.get();
            }
        }
        this->_hasTransparency = result.load();
    }
}
