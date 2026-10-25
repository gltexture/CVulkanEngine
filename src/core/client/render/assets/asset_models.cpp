//
// Created by ded on 30.09.2026.
//

#include "asset_models.h"
#include <filesystem>
#include <fstream>

#include "utility/logger.inl"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image_write.hpp>
#include "client/render/scene/vulkan_render_structs.h"
#include "utility/io_utils.h"

namespace cvulkan::client::render::asset {
    constexpr uint32_t IMPORT_FLAGS = aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FixInfacingNormals | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices;

    void modelGen::to_json(nlohmann::json& json, const CVulkanModelData& model) {
        json = nlohmann::json{
            {"id", model._id},
            {"meshes", model._meshes},
            {"materials_path", model._materialsPath.path().string()},
            {"cache_path", model._cachePath.path().string()},
            {"idx_path", model._idxPath.path().string()},
            {"vtx_path", model._vtxPath.path().string()}
        };
    }

    void modelGen::from_json(const nlohmann::json& json, CVulkanModelData& model) {
        json.at("id").get_to(model._id);
        json.at("meshes").get_to(model._meshes);
        model._materialsPath = io_utils::CVulkanResourcePath{json.at("materials_path").get<std::string>()};
        model._cachePath = io_utils::CVulkanResourcePath{json.at("cache_path").get<std::string>()};
        model._idxPath = io_utils::CVulkanResourcePath{json.at("idx_path").get<std::string>()};
        model._vtxPath = io_utils::CVulkanResourcePath{json.at("vtx_path").get<std::string>()};
    }

    void modelGen::to_json(nlohmann::json& json, const CVulkanMaterialData& material) {
        json = nlohmann::json{
                                    {"id", material._id},
                                    {"texturePath", material._texturePath.path()},
                                    {"diffuseColor", {
                                        material._diffuseColor.x,
                                        material._diffuseColor.y,
                                        material._diffuseColor.z,
                                        material._diffuseColor.w
                                       }}
        };
    }

    void modelGen::from_json(const nlohmann::json& json, CVulkanMaterialData& material) {
        json.at("id").get_to(material._id);
        material._texturePath = io_utils::CVulkanResourcePath{json.at("texturePath").get<std::string>()};
        const auto& color = json.at("diffuseColor");
        material._diffuseColor = glm::vec4{
            color.at(0).get<float>(),
            color.at(1).get<float>(),
            color.at(2).get<float>(),
            color.at(3).get<float>()
        };
    }

    void modelGen::to_json(nlohmann::json& json, const CVulkanMeshData& mesh) {
        json = nlohmann::json{
            {"id", mesh._id},
            {"material_id", mesh._materialId},
            {"vertex_offset", mesh._vertexOffset},
            {"vertex_size", mesh._vtxSize},
            {"index_offset", mesh._indexOffset},
            {"index_size", mesh._idxSize}
        };
    }

    void modelGen::from_json(const nlohmann::json& json, CVulkanMeshData& mesh) {
        json.at("id").get_to(mesh._id);
        json.at("material_id").get_to(mesh._materialId);
        json.at("vertex_offset").get_to(mesh._vertexOffset);
        json.at("vertex_size").get_to(mesh._vtxSize);
        json.at("index_offset").get_to(mesh._indexOffset);
        json.at("index_size").get_to(mesh._idxSize);
    }

    modelGen::CVulkanProcessedModelOutputData modelGen::processRawModelAndGetOutputJSON(const io_utils::CVulkanResourcePath& rawModelRelativeFile) {
        const auto& relativePath = rawModelRelativeFile.path();

        const io_utils::CVulkanResourcePath rawModelResourcePath{io_utils::MODELS_SOURCE / relativePath};
        const auto rawModelFullPath = io_utils::RES_RESOLVER / (rawModelResourcePath);

        const auto normalizedRelativePath = relativePath.lexically_normal();
        if (normalizedRelativePath.empty() || normalizedRelativePath.is_absolute()) {
            throw std::runtime_error("Invalid source model path");
        }
        for (const auto& component : normalizedRelativePath) {
            if (component == "..") {
                throw std::runtime_error("Model path is outside the source models directory");
            }
        }

        const auto rawModelFullPathDir = io_utils::RES_RESOLVER / (io_utils::CVulkanResourcePath{io_utils::MODELS_SOURCE / normalizedRelativePath.parent_path()});
        const io_utils::CVulkanResourcePath outputResourceDir {io_utils::MODELS_COMPILED / normalizedRelativePath.parent_path()};
        const auto outputFullPathDir = io_utils::RES_RESOLVER / (outputResourceDir);

        std::filesystem::remove_all(outputFullPathDir);
        std::filesystem::create_directories(outputFullPathDir);

        logging::info("Processing model '{}'", rawModelFullPath.string());

        Assimp::Importer importer{};

        const aiScene* assimpScene = importer.ReadFile(rawModelFullPath.string(), IMPORT_FLAGS);
        if (assimpScene == nullptr) {
            throw std::runtime_error(std::format("Failed to open scene '{}': {}", rawModelFullPath.string(), importer.GetErrorString()));
        }

        const std::string modelId = normalizedRelativePath.generic_string();

        const std::filesystem::path idxFile = normalizedRelativePath.filename().replace_extension(io_utils::IDX_BIN);
        const std::filesystem::path outIdxPath = outputFullPathDir / idxFile;

        const std::filesystem::path vtxFile = normalizedRelativePath.filename().replace_extension(io_utils::VTX_BIN);
        const std::filesystem::path outVtxPath = outputFullPathDir / vtxFile;

        const std::filesystem::path modelFile = normalizedRelativePath.filename().replace_extension(io_utils::MODEL_JSON);
        const std::filesystem::path outModelFile = outputFullPathDir / modelFile;

        const std::filesystem::path materialFile = normalizedRelativePath.filename().replace_extension(io_utils::MATERIAL_JSON);
        const std::filesystem::path outMaterialFile = outputFullPathDir / materialFile;

        const std::filesystem::path cacheFile = normalizedRelativePath.filename().replace_extension(io_utils::CACHE_BIN);
        const std::filesystem::path outCacheFile = outputFullPathDir / cacheFile;

        CVulkanModelBinData modelBinData{
            ._idxOutput = std::ofstream{outIdxPath, std::ios::out | std::ios::binary | std::ios::trunc},
            ._vtxOutput = std::ofstream{outVtxPath, std::ios::out | std::ios::binary | std::ios::trunc}
        };

        const uint32_t numMaterials = assimpScene->mNumMaterials;
        std::vector<CVulkanMaterialData> materials{};
        materials.reserve(numMaterials);
        for (uint32_t i = 0; i < numMaterials; ++i) {
            const aiMaterial* material = assimpScene->mMaterials[i];
            materials.push_back(processMaterial(*assimpScene, *material, rawModelFullPathDir, {outputFullPathDir / io_utils::TEXTURES}, modelId, i));
        }

        const uint32_t numMeshes = assimpScene->mNumMeshes;
        std::vector<CVulkanMeshData> meshes{};
        meshes.reserve(numMeshes);
        for (uint32_t i = 0; i < numMeshes; ++i) {
            const aiMesh* mesh = assimpScene->mMeshes[i];
            meshes.push_back(processMesh(*assimpScene, *mesh, materials, modelBinData, i));
        }

        {
            const CVulkanModelData modelData{
                ._id = modelId,
                ._meshes = meshes,
                ._materialsPath = io_utils::CVulkanResourcePath{materialFile},
                ._cachePath = io_utils::CVulkanResourcePath{cacheFile},
                ._idxPath = io_utils::CVulkanResourcePath{idxFile},
                ._vtxPath = io_utils::CVulkanResourcePath{vtxFile}
            };
            std::ofstream writer{outModelFile, std::ios::out | std::ios::trunc};
            if (!writer) {
                throw std::runtime_error(std::format("Failed to open output model file '{}'", outModelFile.string()));
            }
            const nlohmann::json json = modelData;
            writer << json.dump(4);
            if (!writer) {
                throw std::runtime_error(std::format("Failed to write output model file '{}'", outModelFile.string()));
            }
            logging::debug("Generated model file '{}'", outModelFile.string());
        }

        {
            std::ofstream writer{outMaterialFile, std::ios::out | std::ios::trunc};
            if (!writer) {
                throw std::runtime_error(std::format("Failed to open output materials file '{}'", outMaterialFile.string()));
            }
            const nlohmann::json json = materials;
            writer << json.dump(4);
            if (!writer) {
                throw std::runtime_error(std::format("Failed to write output materials file '{}'", outMaterialFile.string()));
            }
            logging::debug("Generated materials file '{}'", outMaterialFile.string());
        }

        logging::debug("Generated vertex file '{}'", outVtxPath.string());
        logging::debug("Generated index file '{}'", outIdxPath.string());

        {
            const CVulkanModelCache vulkanModelCache = genModelCache(io_utils::CVulkanResourcePath{normalizedRelativePath.parent_path()});
            std::ofstream writer{outCacheFile, std::ios::out | std::ios::trunc};
            if (!writer) {
                throw std::runtime_error(std::format("Failed to open output cache file '{}'", outCacheFile.string()));
            }
            vulkanModelCache.write(writer);
            logging::debug("Generated cache file '{}'", outCacheFile.string());
        }

        return {
            io_utils::CVulkanResourcePath{io_utils::MODELS_COMPILED} / normalizedRelativePath.parent_path() / modelFile,
            io_utils::CVulkanResourcePath{io_utils::MODELS_COMPILED} / normalizedRelativePath.parent_path() / materialFile
        };
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

    modelGen::CVulkanModelCache modelGen::genModelCache(const io_utils::CVulkanResourcePath& modelRootDirectory) {
        CVulkanModelCache cache {};
        cache._version = _GEN_VER;

        const io_utils::CVulkanResourcePath compiledModelResourcePath = io_utils::CVulkanResourcePath{io_utils::MODELS_COMPILED} / modelRootDirectory;
        const std::filesystem::path modelRootFullPath = io_utils::RES_RESOLVER / compiledModelResourcePath;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(modelRootFullPath)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const auto& path = entry.path();

            CVulkanFileCacheInfo fileCache{};
            fileCache._size = std::filesystem::file_size(path);

            const auto modifiedTime = std::filesystem::last_write_time(path);
            fileCache._modifiedTime = std::chrono::duration_cast<std::chrono::milliseconds>(modifiedTime.time_since_epoch()).count();

            const auto relativePath = path.lexically_relative(modelRootFullPath);
            cache._files.emplace(relativePath, fileCache);
        }
        return cache;
    }

    bool modelGen::isInvalidCache(const io_utils::CVulkanResourcePath& cacheFile, const io_utils::CVulkanResourcePath& modelRootDirectory) {
        const io_utils::CVulkanResourcePath compiledCacheResourcePath = io_utils::CVulkanResourcePath{io_utils::MODELS_COMPILED} / cacheFile;
        const io_utils::CVulkanResourcePath compiledModelResourcePath = io_utils::CVulkanResourcePath{io_utils::MODELS_COMPILED} / modelRootDirectory;

        const std::filesystem::path cacheFullPath = io_utils::RES_RESOLVER / compiledCacheResourcePath;
        const std::filesystem::path modelRootFullPath = io_utils::RES_RESOLVER / compiledModelResourcePath;

        if (!std::filesystem::is_regular_file(cacheFullPath)) {
            return INVALID;
        }
        CVulkanModelCache cache{};
        {
            std::ifstream reader{cacheFullPath, std::ios::binary};
            if (!reader) {
                return INVALID;
            }

            try {
                cache.read(reader);
            } catch (const std::exception&) {
                return INVALID;
            }
        }

        if (cache._version != _GEN_VER) {
            return INVALID;
        }

        std::size_t foundFiles = 0;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(modelRootFullPath)) {
            if (!entry.is_regular_file()) {
                continue;
            }

            const auto& path = entry.path();

            if (path.lexically_normal() == cacheFullPath.lexically_normal()) {
                continue;
            }

            const auto relativePath = path.lexically_relative(modelRootFullPath);
            const auto it = cache._files.find(relativePath);

            if (it == cache._files.end()) {
                return INVALID;
            }

            const auto& info = it->second;

            if (std::filesystem::file_size(path) != info._size) {
                return INVALID;
            }

            const auto modifiedTime = std::filesystem::last_write_time(path);
            const auto modifiedTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(modifiedTime.time_since_epoch()).count();

            if (modifiedTimeMs != info._modifiedTime) {
                return INVALID;
            }

            ++foundFiles;
        }

        return foundFiles != cache._files.size();
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

    modelGen::CVulkanMaterialData modelGen::processMaterial(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& currentPath, const std::filesystem::path& directoryToCopyTextures, const std::string& modelId, uint32_t pos) {
        logging::info("Processing material (model id: '{}'), (material pos: '{}')", modelId, pos);
        aiColor4D color{};
        glm::vec4 colorVec{};
        const aiReturn result = aiGetMaterialColor(&assimpMaterial, AI_MATKEY_COLOR_DIFFUSE, &color);

        if (result == aiReturn_SUCCESS) {
            colorVec.x = color.r;
            colorVec.y = color.g;
            colorVec.z = color.b;
            colorVec.w = color.a;
        }

        const io_utils::CVulkanResourcePath texturePath = processTextureAndGetRelativePath(assimpScene, assimpMaterial, currentPath, directoryToCopyTextures, aiTextureType_DIFFUSE);
        return {std::format("{}-mat-{}", modelId, pos), texturePath, colorVec};
    }

    io_utils::CVulkanResourcePath modelGen::processTextureAndGetRelativePath(const aiScene& assimpScene, const aiMaterial& assimpMaterial, const std::filesystem::path& currentPath, const std::filesystem::path& directoryToCopyTextures, const aiTextureType textureType) {
        const uint32_t numEmbedded = assimpScene.mNumTextures;

        aiString assimpTexturePath{};
        const aiReturn result = aiGetMaterialTexture(&assimpMaterial, textureType, 0, &assimpTexturePath);

        if (result != aiReturn_SUCCESS || assimpTexturePath.Empty()) {
            return io_utils::CVulkanResourcePath {};
        }

        const std::string texturePath = assimpTexturePath.C_Str();

        if (texturePath.starts_with('*')) {
            const uint32_t embeddedTextureIndex = static_cast<uint32_t>(std::stoul(texturePath.substr(1)));
            if (embeddedTextureIndex >= numEmbedded) {
                throw std::runtime_error(std::format("Invalid embedded texture index: {}", embeddedTextureIndex));
            }

            const aiTexture* embeddedTexture = assimpScene.mTextures[embeddedTextureIndex];
            const std::filesystem::path unpackedTexturePath = std::filesystem::path{"unpacked"} / (std::string(embeddedTexture->mFilename.C_Str()) + ".png");
            const std::filesystem::path textureFullPath = directoryToCopyTextures / unpackedTexturePath;

            std::filesystem::create_directories(textureFullPath.parent_path());

            if (embeddedTexture->mHeight == 0) {
                std::ofstream outTexture{textureFullPath,std::ios::out | std::ios::binary | std::ios::trunc};

                if (!outTexture) {
                    throw std::runtime_error(std::format("Failed to open output texture file '{}'", textureFullPath.string()));
                }

                outTexture.write(reinterpret_cast<const char*>(embeddedTexture->pcData), embeddedTexture->mWidth);

                if (!outTexture) {
                    throw std::runtime_error(std::format("Failed to write output texture file '{}'", textureFullPath.string()));
                }
            } else {
                constexpr int channels = 4;
                const int width = static_cast<int>(embeddedTexture->mWidth);
                const int height = static_cast<int>(embeddedTexture->mHeight);
                const int stride = width * channels;

                if (stbi_write_png(textureFullPath.string().c_str(), width, height, channels, embeddedTexture->pcData, stride) == 0) {
                    throw std::runtime_error(std::format("Failed to encode texture '{}' as PNG", textureFullPath.string()));
                }
            }
            logging::debug("Dumped texture '{}'", textureFullPath.string());

            return io_utils::CVulkanResourcePath{io_utils::TEXTURES / unpackedTexturePath};
        }

        const std::filesystem::path source = currentPath / texturePath;

        if (!std::filesystem::is_regular_file(source)) {
            throw std::runtime_error(std::format("Texture file '{}' does not exist", source.string()));
        }

        const std::filesystem::path destination =directoryToCopyTextures / texturePath;
        std::filesystem::create_directories(destination.parent_path());
        std::filesystem::copy_file(source,destination,std::filesystem::copy_options::overwrite_existing);
        return io_utils::CVulkanResourcePath{io_utils::TEXTURES / texturePath};
    }


    std::vector<modelGen::CVulkanMaterialData> modelLoad::loadMaterialsJSON(const io_utils::CVulkanResourcePath& materialsFile) {
        const auto fullPath = io_utils::RES_RESOLVER / materialsFile;
        std::ifstream reader{fullPath, std::ios::in | std::ios::binary};
        if (!reader) {
            throw std::runtime_error(std::format("Failed to open materials file '{}'", fullPath.string()));
        }
        nlohmann::json json;
        reader >> json;
        auto materialData = json.get<std::vector<modelGen::CVulkanMaterialData>>();
        for (auto& material : materialData) {
            material._texturePath = material._texturePath.isEmpty() ? io_utils::CVulkanResourcePath{} : io_utils::CVulkanResourcePath{materialsFile.path().parent_path()} / material._texturePath;
        }
        return materialData;
    }

    modelGen::CVulkanModelData modelLoad::loadModelJSON(const io_utils::CVulkanResourcePath& modelFile) {
        const auto fullPath = io_utils::RES_RESOLVER / modelFile;
        std::ifstream reader{fullPath, std::ios::in | std::ios::binary};
        if (!reader) {
            throw std::runtime_error(std::format("Failed to open model file '{}'", fullPath.string()));
        }
        nlohmann::json json;
        reader >> json;
        auto modelData = json.get<modelGen::CVulkanModelData>();
        const auto parentPath = modelFile.path().parent_path();
        modelData._idxPath = io_utils::CVulkanResourcePath{parentPath / modelData._idxPath.path()};
        modelData._vtxPath = io_utils::CVulkanResourcePath{parentPath / modelData._vtxPath.path()};
        modelData._cachePath = io_utils::CVulkanResourcePath{parentPath / modelData._cachePath.path()};
        modelData._materialsPath = io_utils::CVulkanResourcePath{parentPath / modelData._materialsPath.path()};
        return modelData;
    }

    modelLoad::CVulkanLoadedModelData modelLoad::loadModelWithMaterials(const io_utils::CVulkanResourcePath& modelFile) {
        const auto& relativePath = modelFile.path();
        const bool isModelJSON = relativePath.filename().string().ends_with(io_utils::MODEL_JSON);
        io_utils::CVulkanResourcePath pathToModelJSON = modelFile;

        if (!isModelJSON) {
            auto modelJSONRelativePath = relativePath;
            modelJSONRelativePath.replace_extension(io_utils::MODEL_JSON);
            pathToModelJSON = io_utils::CVulkanResourcePath {io_utils::MODELS_COMPILED / modelJSONRelativePath};

            const auto modelJSONFullPath = io_utils::RES_RESOLVER / (pathToModelJSON);
            bool gen = true;

            if (std::filesystem::is_regular_file(modelJSONFullPath)) {
                const auto modelData = loadModelJSON(pathToModelJSON);
                if (!modelData._cachePath.path().empty()) {
                    const io_utils::CVulkanResourcePath rawModelRoot {io_utils::MODELS_SOURCE / relativePath.parent_path()};
                    gen = modelGen::isInvalidCache(modelData._cachePath, rawModelRoot);
                }
            }

            if (gen) {
                logging::info("Asset '{}' is outdated, recompiling", relativePath.string());
                const auto [outputModelJSONPath, outputMaterialsJSONPath] = modelGen::processRawModelAndGetOutputJSON(modelFile);
                pathToModelJSON = outputModelJSONPath;
            }
        }

        auto modelData = loadModelJSON(pathToModelJSON);
        const auto& pathToMaterialJSON = modelData._materialsPath;

        logging::debug("Loading model '{}' with materials '{}'", pathToModelJSON.path().string(), pathToMaterialJSON.path().string());

        auto materialsData = loadMaterialsJSON(pathToMaterialJSON);
        return {
            ._modelData = std::move(modelData),
            ._materialsData = std::move(materialsData)
        };
    }
}
