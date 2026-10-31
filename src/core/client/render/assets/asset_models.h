//
// Created by ded on 30.09.2026.
//

#pragma once
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <assimp/material.h>

#include "utility/io_utils.h"

struct aiMesh;
struct aiScene;
struct aiMaterial;

namespace cvulkan::client::render::asset {
    inline constexpr uint32_t _GEN_VER = 1;

    constexpr bool INVALID = true;
    constexpr bool VALID = false;

    namespace modelGen {
        struct CVulkanMeshData {
            std::string _id;
            std::string _materialId;
            uint32_t _idxOffset;
            uint32_t _idxSize;
            uint32_t _vtxOffset;
            uint32_t _vtxSize;
        };

        struct CVulkanModelData {
            std::string _id;
            std::vector<CVulkanMeshData> _meshes;
            io_utils::CVulkanResourcePath _materialsPath;
            io_utils::CVulkanResourcePath _cachePath;
            io_utils::CVulkanResourcePath _idxPath;
            io_utils::CVulkanResourcePath _vtxPath;
        };

        struct CVulkanMaterialData {
            std::string _id;
            io_utils::CVulkanResourcePath _texturePath;
            glm::vec4 _diffuseColor;
            bool _hasTransparency;
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

        struct CVulkanFileCacheInfo {
            uint64_t _size;
            int64_t _modifiedTime;
        };

        struct CVulkanModelCache {
            uint32_t _version;
            std::unordered_map<std::filesystem::path, CVulkanFileCacheInfo> _files;

            void write(std::ofstream& writer) const {
                const uint32_t version = _version;
                const uint64_t fileCount = _files.size();
                writer.write(reinterpret_cast<const char*>(&version), sizeof(version));
                writer.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));
                for (const auto& [path, info] : _files) {
                    const std::string pathString = path.generic_string();
                    const uint64_t pathLength = pathString.size();
                    writer.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
                    writer.write(pathString.data(), static_cast<std::streamsize>(pathString.size()));
                    writer.write(reinterpret_cast<const char*>(&info._size), sizeof(info._size));
                    writer.write(reinterpret_cast<const char*>(&info._modifiedTime), sizeof(info._modifiedTime));
                }
                if (!writer) {
                    throw std::runtime_error("Failed to write model cache");
                }
            }

            void read(std::ifstream& input) {
                uint32_t version;
                uint64_t fileCount;
                input.read(reinterpret_cast<char*>(&version), sizeof(version));
                input.read(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
                if (!input) {
                    throw std::runtime_error("Failed to read model cache header");
                }
                this->_version = version;
                this->_files.clear();
                for (uint64_t i = 0; i < fileCount; ++i) {
                    uint64_t pathLength;
                    input.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
                    if (!input) {
                        throw std::runtime_error("Failed to read model cache path length");
                    }
                    if (pathLength > 1024 * 1024) {
                        throw std::runtime_error("Model cache path is too long");
                    }
                    std::string pathString(pathLength, '\0');
                    input.read(pathString.data(), static_cast<std::streamsize>(pathLength));
                    CVulkanFileCacheInfo info{};
                    input.read(reinterpret_cast<char*>(&info._size), sizeof(info._size));
                    input.read(reinterpret_cast<char*>(&info._modifiedTime), sizeof(info._modifiedTime));
                    if (!input) {
                        throw std::runtime_error("Failed to read model cache entry");
                    }
                    this->_files.emplace(std::filesystem::path(pathString), std::move(info));
                }
            }
        };

        struct CVulkanProcessedModelOutputData {
            io_utils::CVulkanResourcePath _outputModelJSONPath;
            io_utils::CVulkanResourcePath _outputMaterialsJSONPath;
        };

        void to_json(nlohmann::json& json, const CVulkanMaterialData& material);
        void from_json(const nlohmann::json& json, CVulkanMaterialData& material);

        void to_json(nlohmann::json& json, const CVulkanMeshData& mesh);
        void from_json(const nlohmann::json& json, CVulkanMeshData& mesh);

        void to_json(nlohmann::json& json, const CVulkanModelData& model);
        void from_json(const nlohmann::json& json, CVulkanModelData& model);

        CVulkanProcessedModelOutputData processRawModelAndGetOutputJSON(const io_utils::CVulkanResourcePath& rawModelRelativeFile);
        CVulkanMaterialData processMaterial(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& currentPath, const std::filesystem::path& directoryToCopyTextures, const std::string& modelId, uint32_t pos);

        CVulkanMeshData processMesh(const aiScene& assimpScene, const aiMesh& assimpMesh, const std::vector<CVulkanMaterialData>& materials, CVulkanModelBinData& modelBinData, uint32_t pos);

        CVulkanModelCache genModelCache(const io_utils::CVulkanResourcePath& rawModelRootDirectory);
        bool isInvalidCache(const io_utils::CVulkanResourcePath& cacheFile, const io_utils::CVulkanResourcePath& modelRootDirectory);

        std::vector<float> processUV(const aiMesh& aiMesh);
        std::vector<uint32_t> processIndices(const aiMesh& aiMesh);
        std::vector<float> processVertices(const aiMesh& aiMesh);

        io_utils::CVulkanResourcePath processTextureAndGetRelativePath(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& currentPath, const std::filesystem::path& directoryToCopyTextures, aiTextureType textureType);
    }

    namespace modelLoad {
        struct CVulkanLoadedModelData {
            modelGen::CVulkanModelData _modelData;
            std::vector<modelGen::CVulkanMaterialData> _materialsData;
        };

        std::vector<modelGen::CVulkanMaterialData> loadMaterialsJSON(const io_utils::CVulkanResourcePath& materialsFile);
        modelGen::CVulkanModelData loadModelJSON(const io_utils::CVulkanResourcePath& modelFile);

        CVulkanLoadedModelData loadModelWithMaterials(const io_utils::CVulkanResourcePath& modelFile);
    }
}
