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
        logging::debug("Loading shader module: '{}'", spvFile.string());

        if (!std::filesystem::exists(spvFile)) {
            throw std::runtime_error(std::format("Shader file does not exist: '{}'", spvFile.string()));
        }
        if (!std::filesystem::is_regular_file(spvFile)) {
            throw std::runtime_error(std::format("Shader path is not a regular file: '{}'", spvFile.string()));
        }

        std::ifstream spv{spvFile, std::ios::binary | std::ios::ate};
        if (!spv) {
            throw std::runtime_error(std::format("Failed to open shader file '{}'", spvFile.string()));
        }

        const std::streamsize fileSize = spv.tellg();
        if (fileSize <= 0) {
            throw std::runtime_error(std::format("Shader file is empty or its size could not be determined: '{}'", spvFile.string()));
        }
        if (fileSize % sizeof(uint32_t) != 0) {
            throw std::runtime_error(std::format("SPIR-V file '{}' has invalid size: {} bytes", spvFile.string(), fileSize));
        }

        spv.seekg(0, std::ios::beg);
        if (!spv) {
            throw std::runtime_error(std::format("Failed to seek to the beginning of shader file '{}'", spvFile.string()));
        }

        std::vector<char> spvFileContents(static_cast<size_t>(fileSize));
        spv.read(spvFileContents.data(), fileSize);
        if (spv.gcount() != fileSize || !spv) {
            throw std::runtime_error(std::format("Failed to read shader file '{}': expected {} bytes, read {} bytes", spvFile.string(), fileSize, spv.gcount()));
        }

        const auto* spvCode = reinterpret_cast<const uint32_t*>(spvFileContents.data());
        if (spvCode[0] != 0x07230203) {
            throw std::runtime_error(std::format("Shader file '{}' has an invalid SPIR-V magic number", spvFile.string()));
        }

        const VkShaderModuleCreateInfo spvModuleCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = static_cast<size_t>(fileSize),
            .pCode = spvCode,
        };

        logging::debug("Creating shader module: stage={}, size={} bytes", static_cast<uint32_t>(this->_stage), fileSize);
        vk::util::vkCheck(vkCreateShaderModule(this->_context.device().vkDevice(), &spvModuleCreateInfo, nullptr, &this->_vkShaderModule), "Failed to create shader module");
        logging::debug("Shader module created successfully: '{}'", spvFile.string());
    }

    CVulkanShaderModule::~CVulkanShaderModule() {
        if (this->_vkShaderModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(this->_context.device().vkDevice(), this->_vkShaderModule, nullptr);
            this->_vkShaderModule = VK_NULL_HANDLE;
        }
    }
}
