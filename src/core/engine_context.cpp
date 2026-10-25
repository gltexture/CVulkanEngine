#include "engine_context.h"

#include "client/client_runtime.h"
#include "utility/logger.inl"
#include "utility/sys_utils.h"

namespace cvulkan {
    EngineContext::EngineContext(EngineData&& engineData) : m_engineData(std::move(engineData)) {
    }

    EngineContext::~EngineContext() {
        globalCleanUp();
    };

    void EngineContext::start() const {
        {
            sys_utils::utilityInitialize();
        }
        logging::info("Initializing application");
        printData(this->m_engineData);
        client::initializeClient(this->m_engineData);
        logging::info("Application initialized");
        client::loop();
    }

    void EngineContext::globalCleanUp() {
        logging::info("Shutting down application");
        client::cleanUp();
    }

    void EngineContext::printData(const EngineData& engineData) {
        logging::info("Engine: {}", engineData.m_appTitle);
    }
}
