//
// Created by ded on 30.09.2026.
//

#pragma once
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <assimp/material.h>

struct aiMesh;
struct aiScene;
struct aiMaterial;

namespace cvulkan::client::render::asset {
    namespace modelGen {
        struct CVulkanMeshData {
            std::string _id;
            std::string _materialId;
            uint32_t _indexOffset;
            uint32_t _idxSize;
            uint32_t _vertexOffset;
            uint32_t _vtxSize;
        };

        struct CVulkanModelData {
            std::string _id;
            std::vector<CVulkanMeshData> _meshes;
            std::filesystem::path _idxPath;
            std::filesystem::path _vtxPath;
        };

        struct CVulkanMaterialData {
            std::string _id;
            std::filesystem::path _texturePath;
            glm::vec4 _diffuseColor;
        };

        struct CVulkanModelBinData {
            std::ofstream _idxOutput;
            std::ofstream _vtxOutput;
            uint32_t _idxOffset{0};
            uint32_t _vtxOffset{0};

            void writeVTX(const float value) {
                _vtxOutput.write(reinterpret_cast<const char*>(&value), sizeof(value));
            }

            void writeIDX(const uint32_t value) {
                _idxOutput.write(reinterpret_cast<const char*>(&value), sizeof(value));
            }
        };


        void to_json(nlohmann::json& json, const CVulkanMaterialData& material);
        void from_json(const nlohmann::json& json, CVulkanMaterialData& material);

        void to_json(nlohmann::json& json, const CVulkanMeshData& mesh);
        void from_json(const nlohmann::json& json, CVulkanMeshData& mesh);

        void to_json(nlohmann::json& json, const CVulkanModelData& model);
        void from_json(const nlohmann::json& json, CVulkanModelData& model);

        void processAndGenModel(const std::filesystem::path& mPath);
        CVulkanMaterialData processMaterial(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& baseDir, const std::string& modelId, uint32_t pos);

        CVulkanMeshData processMesh(const aiScene& assimpScene, const aiMesh& assimpMesh, const std::vector<CVulkanMaterialData>& materials, CVulkanModelBinData& modelBinData, uint32_t pos);

        std::vector<float> processUV(const aiMesh& aiMesh);
        std::vector<uint32_t> processIndices(const aiMesh& aiMesh);
        std::vector<float> processVertices(const aiMesh& aiMesh);

        std::filesystem::path processAndGetTexture(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& baseDir, aiTextureType textureType);
    }

    namespace modelLoad {
        std::vector<modelGen::CVulkanMaterialData> loadMaterials(const std::filesystem::path& path);
        modelGen::CVulkanModelData loadModel(const std::filesystem::path& path);
    }
}
