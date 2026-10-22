//
// Created by ded on 15.09.2026.
//

#include "vulkan_shaders.h"

#include <filesystem>
#include <fstream>

#include "vulkan_render_core.h"
#include "utility/io_utils.h"

namespace cvulkan::client::render::shader {
    CVulkanShaderModule::CVulkanShaderModule(const vk::CVulkanContext& context, const io_utils::CVulkanResourcePath& shaderSpvFileName, const VkShaderStageFlagBits stage) : _context{context}, _stage(stage) {
        const std::filesystem::path spvFile = io_utils::RES_RESOLVER / (io_utils::CVulkanResourcePath{io_utils::SHADERS_COMPILED} / shaderSpvFileName);
        std::ifstream spv{spvFile, std::ios::binary | std::ios::ate};
        if (!spv) {
            throw std::runtime_error(std::format("Failed to open shader file {}", spvFile.string()));
        }
        const std::streamsize fileSize = spv.tellg();
        if (fileSize < 0) {
            throw std::runtime_error(std::format("Failed to determine size of shader file {}", spvFile.string()));
        }
        spv.seekg(0, std::ios::beg);
        std::vector<char> spvFileContents(static_cast<size_t>(fileSize));
        spv.read(spvFileContents.data(), fileSize);
        if (!spv) {
            throw std::runtime_error(std::format("Failed to read shader file {}", spvFile.string()));
        }
        if (fileSize % sizeof(uint32_t) != 0) {
            throw std::runtime_error(std::format("Invalid SPIR-V file size {}", spvFile.string()));
        }
        const VkShaderModuleCreateInfo spvModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = static_cast<size_t>(fileSize),
            .pCode = reinterpret_cast<const uint32_t *>(spvFileContents.data()),
        };
        vk::util::vkCheck(vkCreateShaderModule(this->_context.device().vkDevice(), &spvModuleCreateInfo, nullptr, &this->_vkShaderModule), "Failed to create shader module");
    }

    CVulkanShaderModule::~CVulkanShaderModule() {
        if (this->_vkShaderModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(this->_context.device().vkDevice(), this->_vkShaderModule, nullptr);
            this->_vkShaderModule = VK_NULL_HANDLE;
        }
    }
}
