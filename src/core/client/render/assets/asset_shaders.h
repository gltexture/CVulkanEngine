//
// Created by ded on 30.09.2026.
//

#pragma once

namespace cvulkan::client::render::asset {
    namespace shaderLoad {
        std::vector<char> compileShader(const std::string& shaderName, const std::string& shaderCode, uint32_t shaderType);
        void compileShaderIfOutOfDate(const std::string& shaderName, uint32_t shaderType);
    }
}