#pragma once

#include <string>
#include "glm/vec2.hpp"

namespace cvulkan {
    struct EngineData {
        std::string _appTitle;
        glm::ivec2 _defaultWinSize{1280, 720};
    };

    class CVulkanEngineRunContext {
    public:
        explicit CVulkanEngineRunContext(EngineData&& engineData);

        ~CVulkanEngineRunContext();

        EngineData _engineData;
    };
}