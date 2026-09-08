#include "engine_context.h"

#include "client/client_runtime.h"
#include "client/render/vulkan_utility.h"
#include "util/logger.inl"

namespace cvulkan {
    EngineContext::EngineContext(EngineData&& engineData) : m_engineData(std::move(engineData)) {
    }

    EngineContext::~EngineContext() {
        global_cleanUp();
    };

    void EngineContext::start() const {
        {
            utility::initialize();
        }
        logging::info("Init application:Start");
        printData(this->m_engineData);
        client::initializeClient(this->m_engineData);
        client::loop();
        logging::info("Init application:Complete");
    }

    void EngineContext::global_cleanUp()
    {
        logging::info("Global cleanup!");
        client::cleanUp();
    }

    void EngineContext::printData(const EngineData &engineData) {
        logging::info("Engine : {}", engineData.m_appTitle);
    }
}
