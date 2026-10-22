//
// Created by ded on 22.10.2026.
//

#pragma once
#include <filesystem>

#include "sys_utils.h"

namespace cvulkan::io_utils {
    inline const std::filesystem::path SOURCE{"source"};
    inline const std::filesystem::path COMPILED{"compiled"};
    inline const std::filesystem::path SHADERS{"shaders"};
    inline const std::filesystem::path TEXTURES{"textures"};
    inline const std::filesystem::path MODELS{"models"};
    inline const std::filesystem::path RESOURCES{"resources"};

    inline const std::filesystem::path SHADERS_SOURCE {SHADERS / SOURCE};
    inline const std::filesystem::path SHADERS_COMPILED {SHADERS / COMPILED};

    inline const std::filesystem::path MODELS_COMPILED{MODELS / COMPILED};
    inline const std::filesystem::path MODELS_SOURCE{MODELS / SOURCE};

    inline const std::filesystem::path ERROR_TEXTURE{TEXTURES / "default.png"};

    inline constexpr std::string_view IDX_BIN{"idx.bin"};
    inline constexpr std::string_view VTX_BIN{"vtx.bin"};
    inline constexpr std::string_view CACHE_BIN{"cache.bin"};
    inline constexpr std::string_view MODEL_JSON{"model.json"};
    inline constexpr std::string_view MATERIAL_JSON{"material.json"};

    class CVulkanResourcePath {
    public:
        explicit CVulkanResourcePath() {};

        explicit CVulkanResourcePath(std::filesystem::path path) : _path(std::move(path)) {
            //if (this->_path.empty()) {
            //    throw std::invalid_argument("Resource path is empty");
            //}
            if (this->_path.is_absolute()) {
                throw std::invalid_argument("ResourcePath must be relative");
            }
        }

        [[nodiscard]] CVulkanResourcePath operator/ (const CVulkanResourcePath& other) const {
            return CVulkanResourcePath{_path / other._path};
        }

        [[nodiscard]] CVulkanResourcePath operator/ (const std::filesystem::path& other) const {
            return CVulkanResourcePath{_path / other};
        }

        [[nodiscard]] bool isEmpty() const {
            return _path.empty();
        }

        CVulkanResourcePath(const CVulkanResourcePath&) = default;
        CVulkanResourcePath& operator=(const CVulkanResourcePath&) = default;
        CVulkanResourcePath(CVulkanResourcePath&&) = default;
        CVulkanResourcePath& operator=(CVulkanResourcePath&&) = default;

        [[nodiscard]] const std::filesystem::path& path() const noexcept {
            return _path;
        }

    private:
        std::filesystem::path _path;
    };

    class CVulkanResourceResolver {
    public:
        explicit CVulkanResourceResolver(std::filesystem::path&& root) : _root(std::filesystem::absolute(std::move(root)).lexically_normal()) { }

        [[nodiscard]] std::filesystem::path operator/ (const CVulkanResourcePath& resourcePath) const {
            return resolve(resourcePath);
        }

        CLASS_NO_COPY_NO_MOVE(CVulkanResourceResolver)

        [[nodiscard]] std::filesystem::path resolve(const CVulkanResourcePath& resourcePath) const;

        [[nodiscard]] const std::filesystem::path& root() const noexcept {
            return _root;
        }

    private:
        std::filesystem::path _root;
    };

    std::filesystem::path getExecutableDirectory();

#ifdef ENGINE_DEV_MODE
    inline CVulkanResourceResolver RES_RESOLVER {getExecutableDirectory().parent_path() / RESOURCES};
#else
    inline CVulkanResourceResolver RES_RESOLVER {getExecutableDirectory() / RESOURCES};
#endif
}
