//
// Created by ded on 01.09.2026.
//

#include "vulkan_ext.h"
#include "util/logger.h"

namespace cvulkan::client::renderCore::ext {
    static constexpr VkDebugUtilsMessageSeverityFlagsEXT MESS_SEVERITY_BIT =
      //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    static constexpr VkDebugUtilsMessageTypeFlagsEXT MESS_TYPE_BIT =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    static constexpr auto DEBUG_CALL_BACK_PREF = "VkDebugUtilsCallback, {}";

    static VkBool32 VKAPI_PTR debugVkCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
            logging::info(DEBUG_CALL_BACK_PREF, pCallbackData->pMessage);
        } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
            logging::warn(DEBUG_CALL_BACK_PREF, pCallbackData->pMessage);
        } else if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
            logging::error(DEBUG_CALL_BACK_PREF, pCallbackData->pMessage);
        } else {
            logging::debug(DEBUG_CALL_BACK_PREF, pCallbackData->pMessage);
        }
        return VK_FALSE;
    }

    VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerCreateInfo() {
        VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
        messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        messengerCreateInfo.messageType = MESS_TYPE_BIT;
        messengerCreateInfo.messageSeverity = MESS_SEVERITY_BIT;
        messengerCreateInfo.pfnUserCallback = &debugVkCallback;
        return messengerCreateInfo;
    }
}
