//
// Created by ded on 08.10.2026.
//

#pragma once
#include <filesystem>

#include "utility/sys_utils.h"

namespace cvulkan::client::render::vk {
    class CVulkanCommandBuffer;
    class CVulkanContext;
    class CVulkanBuffer;
    class CVulkanImage2DView;
    class CVulkanImage2D;
}

namespace cvulkan::client::render::asset {
    struct CVulkanImageSrc {
        ~CVulkanImageSrc();

        unsigned char*& _data;
        size_t _dataSize;
        glm::vec2 _size;
        uint32_t _channels;
    };

    class CVulkanTexture2D {
    public:
        CVulkanTexture2D(const vk::CVulkanContext& context, const std::string& id, const CVulkanImageSrc& imageSrc, const VkFormat& format);
        ~CVulkanTexture2D();

        CLASS_NO_COPY(CVulkanTexture2D);
        CVulkanTexture2D(CVulkanTexture2D&& other) noexcept;
        CVulkanTexture2D& operator=(CVulkanTexture2D&& other) noexcept;

        void recordTextureTransition(const vk::CVulkanCommandBuffer& commandBuffer);
        void recordCopyBuffer(const vk::CVulkanCommandBuffer& commandBuffer, const vk::CVulkanBuffer& bufferData) const;

        void destroyStgBuffer();

        [[nodiscard]] const std::string& id() const {
            return _id;
        }

        [[nodiscard]] const glm::vec2& size() const {
            return _size;
        }

        [[nodiscard]] bool recordedTransitionFlag() const {
            return _recordedTransition;
        }

        [[nodiscard]] const vk::CVulkanImage2D* image() const {
            return _image.get();
        }

        [[nodiscard]] const vk::CVulkanImage2DView* imageView() const {
            return _imageView.get();
        }

        [[nodiscard]] const vk::CVulkanBuffer* stgBuffer() const {
            return _stgBuffer.get();
        }

    protected:
        void createStgBuffer(const vk::CVulkanContext& context, unsigned char*& _data, const size_t& _dataSize);

    private:
        std::string _id;
        glm::vec2 _size;
        bool _recordedTransition {false};
        std::unique_ptr<vk::CVulkanImage2D> _image;
        std::unique_ptr<vk::CVulkanImage2DView> _imageView;
        std::unique_ptr<vk::CVulkanBuffer> _stgBuffer;
    };


    namespace imageLoad {
        CVulkanImageSrc loadImage(const std::filesystem::path& imagePath);
    }
}
