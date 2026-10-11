//
// Created by ded on 30.09.2026.
//

#include "asset_models.h"
#include <filesystem>
#include <fstream>

#include "util/logger.inl"
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "client/render/scene/vulkan_render_structs.h"

namespace cvulkan::client::render::asset {
    constexpr uint32_t IMPORT_FLAGS = aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices;

    void to_json(nlohmann::json& json, const modelGen::CVulkanModelData& model) {
        json = nlohmann::json{
                    {"id", model._id},
                    {"meshes", model._meshes},
                    {"idx_path", model._idxPath.string()},
                    {"vtx_path", model._vtxPath.string()}
        };
    }

    void from_json(const nlohmann::json& json, modelGen::CVulkanModelData& model) {
        json.at("id").get_to(model._id);
        json.at("meshes").get_to(model._meshes);
        model._idxPath = {json.at("idx_path").get<std::string>()};
        model._vtxPath = {json.at("vtx_path").get<std::string>()};
    }

    void to_json(nlohmann::json& json, const modelGen::CVulkanMaterialData& material) {
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

    void from_json(const nlohmann::json& json, modelGen::CVulkanMaterialData& material) {
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

    void to_json(nlohmann::json& json, const modelGen::CVulkanMeshData& mesh) {
        json = nlohmann::json{
                    {"id", mesh._id},
                    {"material_id", mesh._materialId},
                    {"vertex_offset", mesh._vertexOffset},
                    {"vertex_size", mesh._vtxSize},
                    {"index_offset", mesh._indexOffset},
                    {"index_size", mesh._idxSize}
        };
    }

    void from_json(const nlohmann::json& json, modelGen::CVulkanMeshData& mesh) {
        json.at("id").get_to(mesh._id);
        json.at("material_id").get_to(mesh._materialId);
        json.at("vertex_offset").get_to(mesh._vertexOffset);
        json.at("vertex_size").get_to(mesh._vtxSize);
        json.at("index_offset").get_to(mesh._indexOffset);
        json.at("index_size").get_to(mesh._idxSize);
    }

    void modelGen::processAndGenModel(const std::filesystem::path& mPath) {
        logging::info("Processing model {}", mPath.string());
        if (!std::filesystem::is_regular_file(mPath)) {
            throw std::runtime_error(std::format("Model {} does not exist", mPath.string()));
        }

        Assimp::Importer importer{};

        const aiScene* assimpScene = importer.ReadFile(mPath.string(), IMPORT_FLAGS);
        if (assimpScene == nullptr) {
            throw std::runtime_error(std::format("Failed to open scene {}: {}", mPath.string(), importer.GetErrorString()));
        }

        const std::string modelId = mPath.stem().string();

        const std::filesystem::path outIdxPath = std::filesystem::path{mPath}.replace_extension(".idx");
        const std::filesystem::path outVtxPath = std::filesystem::path{mPath}.replace_extension(".vtx");

        CVulkanModelBinData modelBinData {
            ._idxOutput = std::ofstream{outIdxPath, std::ios::out | std::ios::binary | std::ios::trunc},
            ._vtxOutput = std::ofstream{outVtxPath, std::ios::out | std::ios::binary | std::ios::trunc}
        };

        const uint32_t numMaterials = assimpScene->mNumMaterials;
        logging::debug("Num Materials: {}", numMaterials);
        std::vector<CVulkanMaterialData> materials{};
        materials.reserve(numMaterials);
        const std::filesystem::path materialParentPath{mPath.parent_path()};
        for (uint32_t i = 0; i < numMaterials; ++i) {
            const aiMaterial* material = assimpScene->mMaterials[i];
            materials.push_back(processMaterial(*assimpScene, *material, materialParentPath, modelId, i));
        }

        const uint32_t numMeshes = assimpScene->mNumMeshes;
        std::vector<CVulkanMeshData> meshes{};
        meshes.reserve(numMeshes);
        for (uint32_t i = 0; i < numMeshes; ++i) {
            const aiMesh* mesh = assimpScene->mMeshes[i];
            meshes.push_back(processMesh(*assimpScene, *mesh, materials, modelBinData, i));
        }

        {
            const CVulkanModelData modelData {
                ._id = modelId,
                ._meshes = meshes,
                ._idxPath = outIdxPath,
                ._vtxPath = outVtxPath,
            };
            const std::filesystem::path outModelFile = std::filesystem::path{mPath}.replace_extension(".json");
            std::ofstream writer{outModelFile, std::ios::out | std::ios::trunc};
            if (!writer) {
                throw std::runtime_error(std::format("Failed to open model file {}",outModelFile.string()));
            }
            const nlohmann::json json = modelData;
            writer << json.dump(4);
            if (!writer) {
                throw std::runtime_error(std::format("Failed to write model file {}", outModelFile.string()));
            }
            logging::info("Generated model file [{}]", outModelFile.string());
        }

        {
            const std::filesystem::path outMaterialFile = std::filesystem::path{mPath}.replace_extension("_mat.json");
            std::ofstream writer{outMaterialFile, std::ios::out | std::ios::trunc};
            if (!writer) {
                throw std::runtime_error(std::format("Failed to open materials file {}", outMaterialFile.string()));
            }
            const nlohmann::json json = materials;
            writer << json.dump(4);
            if (!writer) {
                throw std::runtime_error(std::format("Failed to write materials file {}", outMaterialFile.string()));
            }
            logging::info("Generated materials file [{}]", outMaterialFile.string());
        }

        logging::info("Generated vtx file {}", outVtxPath.string());
        logging::info("Generated idx file {}", outIdxPath.string());
    }

    modelGen::CVulkanMeshData modelGen::processMesh(const aiScene& assimpScene, const aiMesh& assimpMesh, const std::vector<CVulkanMaterialData>& materials, CVulkanModelBinData& modelBinData, uint32_t pos) {
        const std::vector<float> vertices = processVertices(assimpMesh);
        std::vector<float> uv = processUV(assimpMesh);
        const std::vector<uint32_t> indices = processIndices(assimpMesh);

        const size_t vertSize = vertices.size();
        if (uv.empty()) {
            uv.resize(vertSize / 3 * 2);
        }

        const uint32_t rows = vertSize / 3;
        const uint32_t vtxInc = (vertSize + uv.size()) * sizeof(float);
        for (uint32_t row = 0; row < rows; ++row) {
            const uint32_t startPos = row * 3;
            const uint32_t startUV = row * 2;

            modelBinData.writeVTX(vertices[startPos]);
            modelBinData.writeVTX(vertices[startPos + 1]);
            modelBinData.writeVTX(vertices[startPos + 2]);
            modelBinData.writeVTX(uv[startUV]);
            modelBinData.writeVTX(uv[startUV + 1]);
        }

        const size_t idxSize = indices.size();
        const uint32_t idxInc = idxSize * sizeof(uint32_t);
        for (uint32_t idx = 0; idx < idxSize; ++idx) {
            modelBinData.writeIDX(indices[idx]);
        }

        const std::string id = std::format("{}_{}", assimpMesh.mName.C_Str(), pos);
        const uint32_t materialIdx = assimpMesh.mMaterialIndex;
        std::string materialId{};
        if (materialIdx < materials.size()) {
            materialId = materials[materialIdx]._id;
        }

        CVulkanMeshData meshData {
            ._id = id,
            ._materialId = materialId,
            ._indexOffset = modelBinData._idxOffset,
            ._idxSize = idxInc,
            ._vertexOffset = modelBinData._vtxOffset,
            ._vtxSize = vtxInc,
        };

        modelBinData._vtxOffset += vtxInc;
        modelBinData._idxOffset += idxInc;

        return meshData;
    }

    std::vector<float> modelGen::processUV(const aiMesh& aiMesh) {
        std::vector<float> textureCoords;
        if (aiMesh.mTextureCoords[0] == nullptr) {
            return textureCoords;
        }
        textureCoords.reserve(aiMesh.mNumVertices * 2);
        for (uint32_t i = 0; i < aiMesh.mNumVertices; ++i) {
            const aiVector3D& textureCoord = aiMesh.mTextureCoords[0][i];
            textureCoords.push_back(textureCoord.x);
            textureCoords.push_back(1.0f - textureCoord.y);
        }
        return textureCoords;
    }

    std::vector<uint32_t> modelGen::processIndices(const aiMesh& aiMesh) {
        std::vector<uint32_t> indices;
        for (uint32_t i = 0; i < aiMesh.mNumFaces; ++i) {
            const aiFace& face = aiMesh.mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }
        return indices;
    }

    std::vector<float> modelGen::processVertices(const aiMesh& aiMesh) {
        std::vector<float> vertices;
        vertices.reserve(aiMesh.mNumVertices * 3);
        for (uint32_t i = 0; i < aiMesh.mNumVertices; ++i) {
            const aiVector3D& vertex = aiMesh.mVertices[i];
            vertices.push_back(vertex.x);
            vertices.push_back(vertex.y);
            vertices.push_back(vertex.z);
        }
        return vertices;
    }

    modelGen::CVulkanMaterialData modelGen::processMaterial(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& baseDir, const std::string& modelId, uint32_t pos) {
        logging::debug("Processing material {}", modelId);
        aiColor4D color{};
        glm::vec4 colorVec{};
        const aiReturn result = aiGetMaterialColor(&assimpMaterial, AI_MATKEY_COLOR_DIFFUSE, &color);

        if (result == aiReturn_SUCCESS) {
            colorVec.x = color.r;
            colorVec.y = color.g;
            colorVec.z = color.b;
            colorVec.w = color.a;
        }

        const std::filesystem::path texturePath = processAndGetTexture(assimpScene, assimpMaterial, baseDir, aiTextureType_DIFFUSE);
        return {std::format("{}-mat-{}", modelId, pos), texturePath, colorVec};
    }

    std::filesystem::path modelGen::processAndGetTexture(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& baseDir, const aiTextureType textureType) {
        const uint32_t numEmbedded = assimpScene.mNumTextures;
        aiString assimpTexturePath{};
        const aiReturn result = aiGetMaterialTexture(&assimpMaterial, textureType, 0, &assimpTexturePath);
        if (result == aiReturn_SUCCESS && !assimpTexturePath.Empty()) {
            const std::string texturePath = assimpTexturePath.C_Str();
            if (texturePath.starts_with('*')) {
                const uint32_t embeddedTextureIndex = static_cast<uint32_t>(std::stoul(texturePath.substr(1)));
                if (embeddedTextureIndex >= numEmbedded) {
                    throw std::runtime_error(std::format("Invalid embedded texture index: {}", embeddedTextureIndex));
                }

                const aiTexture* embeddedTexture = assimpScene.mTextures[embeddedTextureIndex];
                const std::filesystem::path textureFullPath{baseDir / (std::string(embeddedTexture->mFilename.C_Str()) + ".png")};
                logging::info("Dumped texture: {}", textureFullPath.string());
                std::ofstream outTexture {textureFullPath, std::ios::out | std::ios::binary | std::ios::trunc};

                {
                    if (embeddedTexture->mHeight == 0) {
                        outTexture.write(reinterpret_cast<const char*>(embeddedTexture->pcData), embeddedTexture->mWidth);
                    } else {
                        throw std::runtime_error("TODO"); // TODO
                    }
                }

                if (!outTexture) {
                    throw std::runtime_error(std::format("Failed to write texture file {}", textureFullPath.string()));
                }

                return textureFullPath;
            }
            return texturePath;
        }
        throw std::runtime_error("Couldn't get aiMaterialTexture");
    }


    std::vector<modelGen::CVulkanMaterialData> modelLoad::loadMaterials(const std::filesystem::path& path) {
        logging::info("Loading materials from {}", path.string());
        std::ifstream reader{path, std::ios::in | std::ios::binary};
        if (!reader) {
            throw std::runtime_error(std::format("Failed to open materials file {}", path.string()));
        }
        nlohmann::json json;
        reader >> json;
        return json.get<std::vector<modelGen::CVulkanMaterialData>>();
    }

    modelGen::CVulkanModelData modelLoad::loadModel(const std::filesystem::path& path) {
        logging::info("Loading model from {}", path.string());
        std::ifstream reader{path, std::ios::in | std::ios::binary};
        if (!reader) {
            throw std::runtime_error(std::format("Failed to open model file {}", path.string()));
        }
        nlohmann::json json;
        reader >> json;
        return json.get<modelGen::CVulkanModelData>();
    }
}
