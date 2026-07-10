#pragma once
#include <string>

#include "glm/vec2.hpp"

namespace cvulkan {
    struct EngineData
    {
        std::string m_appTitle;
        glm::ivec2 m_default_windowSize { 1280, 720 };
    };

    class EngineContext
    {
    public:
        explicit EngineContext(EngineData&& engineData);
        ~EngineContext();

        EngineData m_engineData;

        void start() const;

        static void global_cleanUp();
        static void printData(const EngineData& engineData);
    };
}