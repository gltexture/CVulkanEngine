#include "vulkan_utility.h"

#include "util/logger.inl"
#include <vulkan/vk_enum_string_helper.h>

namespace cvulkan::utility {
    void vkCheck(const VkResult vk_result, std::string_view errMsg) {
        if (vk_result != VK_SUCCESS) {
            throw std::runtime_error{
                std::format(
                    "{} -> {}",
                    string_VkResult(vk_result),
                    errMsg
                )
            };
        }
    }

    void vkCheck(const VkResult vk_result) {
        vkCheck(vk_result, "Unknown");
    }

    void utilityInitialize() {
        if constexpr (debug_mode) {
            spdlog::set_level(spdlog::level::debug);
            logging::debug("DEBUG MODE");
        }
    }

    void imageBarrier(const VkCommandBuffer commandBuffer, const VkImage image, const VkImageLayout oldLayout, const VkImageLayout newLayout, const VkPipelineStageFlags2 srcStageMask, const VkPipelineStageFlags2 dstStageMask,
                      const VkAccessFlags2 srcAccessMask, const VkAccessFlags2 dstAccessMask, const VkImageAspectFlags aspectMask) {
        const VkImageMemoryBarrier2 barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = srcStageMask,
            .srcAccessMask = srcAccessMask,
            .dstStageMask = dstStageMask,
            .dstAccessMask = dstAccessMask,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = aspectMask,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            },
        };

        const VkDependencyInfo dependencyInfo = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier,
        };

        vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
    }
}
