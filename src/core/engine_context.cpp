#include "engine_context.h"

#include "client/client_runtime.h"
#include "util/logger.inl"

namespace cvulkan {
    EngineContext::EngineContext(EngineData&& engineData) {
        this->m_engineData = std::move(engineData);
    }

    EngineContext::~EngineContext() {
        global_cleanUp();
    };

    void EngineContext::start() const {
        logging::info("Init application:Start");
        printData(this->m_engineData);
        {
            client::initializeClient(this->m_engineData);
            client::loop();
        }
        logging::info("Init application:Complete");
    }

    void EngineContext::global_cleanUp()
    {
        client::cleanUp();
    }

    void EngineContext::printData(const EngineData &engineData) {
        logging::info("Engine : {}", engineData.m_appTitle);
    }
}
