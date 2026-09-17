//
// Created by ded on 15.09.2026.
//

#include "vulkan_shaders.h"

#include <filesystem>
#include <fstream>

#include "vulkan_render_core.h"

namespace cvulkan::client::render::shader {
    void CVulkanShaderModule::createShaderModule(const std::string_view shaderSpvFileName) {
        std::ifstream spv {std::string(shaderSpvFileName), std::ios::binary | std::ios::ate};
        if (!spv) {
            throw std::runtime_error(std::format("Failed to open shader file {}", shaderSpvFileName));
        }
        const std::streamsize fileSize = spv.tellg();
        if (fileSize < 0) {
            throw std::runtime_error(std::format("Failed to determine size of shader file {}", shaderSpvFileName));
        }
        spv.seekg(0, std::ios::beg);
        std::vector<char> spvFileContents(static_cast<size_t>(fileSize));
        spv.read(spvFileContents.data(), fileSize);
        if (!spv) {
            throw std::runtime_error(std::format("Failed to read shader file {}", shaderSpvFileName));
        }
        if (fileSize % sizeof(uint32_t) != 0) {
            throw std::runtime_error(std::format("Invalid SPIR-V file size {}", shaderSpvFileName));
        }
        const VkShaderModuleCreateInfo spvModuleCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = static_cast<size_t>(fileSize),
            .pCode = reinterpret_cast<const uint32_t*>(spvFileContents.data()),
        };
        utility::vkCheck(vkCreateShaderModule(this->_context.device().vkDevice, &spvModuleCreateInfo, nullptr, &this->_vkShaderModule), "Failed to create shader module");
    }

    void CVulkanShaderModule::destroyShaderModule() {
        if (this->_vkShaderModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(this->_context.device().vkDevice, this->_vkShaderModule, nullptr);
            this->_vkShaderModule = VK_NULL_HANDLE;
        }
    }

    std::vector<char> compileShader(const std::string& shaderName, const std::string& shaderCode, const shaderc_shader_kind shaderType) {
        const shaderc_compiler_t compiler = shaderc_compiler_initialize();
        const shaderc_compile_options_t options = shaderc_compile_options_initialize();
        if (utility::debug_mode) {
            shaderc_compile_options_set_generate_debug_info(options);
            shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_zero);
            shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
        }

        const shaderc_compilation_result_t compiled = shaderc_compile_into_spv(compiler, shaderCode.data(), shaderCode.size(), shaderType, shaderName.data(), "main", options);
        if (shaderc_result_get_compilation_status(compiled) != shaderc_compilation_status_success) {
            const char* errorMessage = shaderc_result_get_error_message(compiled);
            shaderc_result_release(compiled);
            shaderc_compile_options_release(options);
            shaderc_compiler_release(compiler);
            throw std::runtime_error(std::format("Shader compilation failed: {}",errorMessage));
        }

        const size_t byteCount = shaderc_result_get_length(compiled);
        const char* bytes = shaderc_result_get_bytes(compiled);

        std::vector compiledShader(bytes,bytes + byteCount);

        shaderc_result_release(compiled);
        shaderc_compile_options_release(options);
        shaderc_compiler_release(compiler);

        return compiledShader;
    }

    void compileShaderIfOutOfDate(const std::string& shaderName, const shaderc_shader_kind shaderType) {
        const std::filesystem::path glslFile {utility::SHADERS_FOLDER / shaderName};
        const std::filesystem::path spvFile {utility::SHADERS_FOLDER_SPV / (std::string{shaderName} + ".spv")};

        if (!std::filesystem::exists(spvFile) || (std::filesystem::last_write_time(glslFile) > std::filesystem::last_write_time(spvFile))) {
            std::ifstream fileShaderCode {glslFile, std::ios::in};
            if (!fileShaderCode) {
                throw std::runtime_error(std::format("Failed to open glsl file {}", glslFile.string()));
            }
            const std::string shaderCode {
                std::istreambuf_iterator<char> {fileShaderCode},
                std::istreambuf_iterator<char> {}
            };
            const std::vector<char> compiledBytes = compileShader(shaderName, shaderCode, shaderType);
            std::ofstream file {spvFile, std::ios::trunc | std::ios::out | std::ios::binary};
            if (!file) {
                throw std::runtime_error(std::format("Failed to open SPIR-V file {}", spvFile.string()));
            }
            file.write(compiledBytes.data(), compiledBytes.size());
            if (!file) {
                throw std::runtime_error(std::format("Failed to write SPIR-V file {}", spvFile.string()));
            }
            logging::debug("Compiled shader {}", shaderName);
        } else {
            logging::debug("Shader {} already compiled, skipping", shaderName);
        }
    }
}
