//
// Created by ded on 30.09.2026.
//

#include "asset_shaders.h"

#include <fstream>


#include "../../../utility/logger.h"
#include "utility/io_utils.h"
#include "utility/sys_utils.h"

namespace cvulkan::client::render::asset {
    std::vector<char> shaderCompile::compileShader(const std::string& shaderName, const std::string& shaderCode, uint32_t shaderType) {
        const shaderc_compiler_t compiler = shaderc_compiler_initialize();
        const shaderc_compile_options_t options = shaderc_compile_options_initialize();
        if (sys_utils::debug_mode) {
            shaderc_compile_options_set_generate_debug_info(options);
            shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_zero);
            shaderc_compile_options_set_source_language(options, shaderc_source_language_glsl);
        }

        const shaderc_compilation_result_t compiled = shaderc_compile_into_spv(compiler, shaderCode.data(), shaderCode.size(), static_cast<shaderc_shader_kind>(shaderType), shaderName.data(), "main", options);
        if (shaderc_result_get_compilation_status(compiled) != shaderc_compilation_status_success) {
            const char* errorMessage = shaderc_result_get_error_message(compiled);
            shaderc_result_release(compiled);
            shaderc_compile_options_release(options);
            shaderc_compiler_release(compiler);
            throw std::runtime_error(std::format("Failed to compile shader: {}", errorMessage));
        }

        const size_t byteCount = shaderc_result_get_length(compiled);
        const char* bytes = shaderc_result_get_bytes(compiled);

        std::vector compiledShader(bytes, bytes + byteCount);

        shaderc_result_release(compiled);
        shaderc_compile_options_release(options);
        shaderc_compiler_release(compiler);

        return compiledShader;
    }

    void shaderCompile::compileShaderIfOutOfDate(const io_utils::CVulkanResourcePath& shaderSourceFile, const uint32_t shaderType) {
        const std::filesystem::path relativePath = shaderSourceFile.path();
        std::filesystem::path spvRelativePath = relativePath;
        spvRelativePath += ".spv";

        const std::filesystem::path glslFile = io_utils::RES_RESOLVER / io_utils::CVulkanResourcePath{io_utils::SHADERS_SOURCE / relativePath};
        const std::filesystem::path spvFile = io_utils::RES_RESOLVER / io_utils::CVulkanResourcePath{io_utils::SHADERS_COMPILED / spvRelativePath};

        if (!std::filesystem::is_regular_file(glslFile)) {
            throw std::runtime_error(std::format("Shader source file '{}' does not exist", glslFile.string()));
        }

        if (!std::filesystem::is_regular_file(spvFile) || (std::filesystem::last_write_time(glslFile) > std::filesystem::last_write_time(spvFile))) {
            std::ifstream fileShaderCode{glslFile, std::ios::in};
            if (!fileShaderCode) {
                throw std::runtime_error(std::format("Failed to open GLSL file '{}'", glslFile.string()));
            }
            const std::string shaderCode{
                std::istreambuf_iterator<char>{fileShaderCode},
                std::istreambuf_iterator<char>{}
            };
            const std::vector<char> compiledBytes = compileShader(spvRelativePath.filename().string(), shaderCode, shaderType);
            std::filesystem::create_directories(spvFile.parent_path());
            std::ofstream file{spvFile, std::ios::trunc | std::ios::out | std::ios::binary};
            if (!file) {
                throw std::runtime_error(std::format("Failed to open SPIR-V file '{}'", spvFile.string()));
            }
            file.write(compiledBytes.data(), static_cast<std::streamsize>(compiledBytes.size()));
            if (!file) {
                throw std::runtime_error(std::format("Failed to write SPIR-V file '{}'", spvFile.string()));
            }
            logging::info("Compiled shader '{}'", spvRelativePath.filename().string());
        } else {
            logging::info("Shader '{}' is up to date, skipping compilation", spvRelativePath.filename().string());
        }
    }

    shader::CVulkanShaderModule shaderLoad::compileAndLoadShader(const vk::CVulkanContext& context, const io_utils::CVulkanResourcePath& shaderSourceFile, const uint32_t shaderType, const VkShaderStageFlagBits vkStage) {
        shaderCompile::compileShaderIfOutOfDate(shaderSourceFile, shaderType);

        const std::filesystem::path relativePath = shaderSourceFile.path();
        std::filesystem::path spvRelativePath = relativePath;
        spvRelativePath += ".spv";
        return {context, io_utils::CVulkanResourcePath{spvRelativePath}, vkStage};
    }
}
