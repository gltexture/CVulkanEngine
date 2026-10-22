#include "engine_context.h"

#include "client/client_runtime.h"
#include "utility/logger.inl"
#include "utility/sys_utils.h"

namespace cvulkan {
    EngineContext::EngineContext(EngineData&& engineData) : m_engineData(std::move(engineData)) {
    }

    EngineContext::~EngineContext() {
        global_cleanUp();
    };

    void EngineContext::start() const {
        {
            sys_utils::utilityInitialize();
        }
        logging::info("Init application:Start");
        printData(this->m_engineData);
        client::initializeClient(this->m_engineData);
        client::loop();
        logging::info("Init application:Complete");
    }

    void EngineContext::global_cleanUp() {
        logging::info("Global cleanup!");
        client::cleanUp();
    }

    void EngineContext::printData(const EngineData& engineData) {
        logging::info("Engine : {}", engineData.m_appTitle);
    }
}
