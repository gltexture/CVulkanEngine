//
// Created by ded on 30.09.2026.
//

#pragma once
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

struct aiMesh;
enum aiTextureType : int;
struct aiScene;
struct aiMaterial;

namespace cvulkan::client::render::asset {
    struct CVulkanMaterialData {
        std::string _id;
        std::filesystem::path _texturePath;
        glm::vec4 _diffuseColor;
    };

    inline void to_json(nlohmann::json& json, const CVulkanMaterialData& material) {
        json = nlohmann::json{
                {"id", material._id},
                {"texturePath", material._texturePath},
                {"diffuseColor", {
                    material._diffuseColor.x,
                    material._diffuseColor.y,
                    material._diffuseColor.z,
                    material._diffuseColor.w
                   }}
        };
    }

    inline void from_json(const nlohmann::json& json, CVulkanMaterialData& material) {
        json.at("id").get_to(material._id);
        json.at("texture_path").get_to(material._texturePath);
        const auto& color = json.at("diffuse_color");
        material._diffuseColor = glm::vec4{
            color.at(0).get<float>(),
            color.at(1).get<float>(),
            color.at(2).get<float>(),
            color.at(3).get<float>()
        };
    }

    struct CVulkanMeshData {
        std::string _id;
        std::string _materialId;
        uint32_t _indexOffset;
        uint32_t _indexSize;
        uint32_t _vertexOffset;
        uint32_t _vertexSize;
    };

    inline void to_json(nlohmann::json& json, const CVulkanMeshData& mesh) {
        json = nlohmann::json{
            {"id", mesh._id},
            {"material_id", mesh._materialId},
            {"vertex_offset", mesh._vertexOffset},
            {"vertex_size", mesh._vertexSize},
            {"index_offset", mesh._indexOffset},
            {"index_size", mesh._indexSize}
        };
    }

    inline void from_json(const nlohmann::json& json, CVulkanMeshData& mesh) {
        json.at("id").get_to(mesh._id);
        json.at("material_id").get_to(mesh._materialId);
        json.at("vertex_offset").get_to(mesh._vertexOffset);
        json.at("vertex_size").get_to(mesh._vertexSize);
        json.at("index_offset").get_to(mesh._indexOffset);
        json.at("index_size").get_to(mesh._indexSize);
    }

    struct CVulkanModelData {
        std::string _id;
        std::vector<CVulkanMeshData> _meshes;
        std::filesystem::path _idxPath;
        std::filesystem::path _vtxPath;
    };

    inline void to_json(nlohmann::json& json, const CVulkanModelData& model) {
        json = nlohmann::json{
            {"id", model._id},
            {"meshes", model._meshes},
            {"idx_path", model._idxPath.string()},
            {"vtx_path", model._vtxPath.string()}
        };
    }

    inline void from_json(const nlohmann::json& json, CVulkanModelData& model) {
        json.at("id").get_to(model._id);
        json.at("meshes").get_to(model._meshes);
        model._idxPath = static_cast<std::filesystem::path>(json.at("idx_path").get<std::string>());
        model._vtxPath = static_cast<std::filesystem::path>(json.at("vtx_path").get<std::string>());
    }

    struct CVulkanModelBinData {
        std::ofstream _idxOutput;
        std::ofstream _vtxOutput;
        uint32_t _idxOffset{0};
        uint32_t _vtxOffset{0};

        void writeVTX(const float value) {
            _vtxOutput.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }

        void writeIDX(const float value) {
            _idxOutput.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
    };

    namespace modelGen {
        void processAndGenModel(const std::filesystem::path& mPath);
        CVulkanMaterialData processMaterial(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& baseDir, const std::string& modelId, uint32_t pos);
        CVulkanMeshData processMesh(const aiScene& assimpScene, const aiMesh& assimpMesh, const std::vector<CVulkanMaterialData>& materials, CVulkanModelBinData& modelBinData, uint32_t pos);

        std::vector<float> processUV(const aiMesh& aiMesh);
        std::vector<uint32_t> processIndices(const aiMesh& aiMesh);
        std::vector<float> processVertices(const aiMesh& aiMesh);

        std::filesystem::path processAndGetTexture(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& baseDir, aiTextureType textureType);
    }

    namespace modelLoad {
        std::vector<CVulkanMaterialData> loadMaterials(const std::filesystem::path& path);
        CVulkanModelData loadModel(const std::filesystem::path& path);
    }
}
