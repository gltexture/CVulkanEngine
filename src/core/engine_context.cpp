#include "engine_context.h"

#include "client/client_runtime.h"
#include "utility/logging.inl"

namespace cvulkan {
    CVulkanEngineRunContext::CVulkanEngineRunContext(EngineData&& engineData) : _engineData(std::move(engineData)) {
        {
            log::init();
        }
        log::info("Initializing application");
        log::info("Engine: {}", _engineData._appTitle);
        client::initializeClient(this->_engineData);
        log::info("Application initialized");
        client::loop();
    }

    CVulkanEngineRunContext::~CVulkanEngineRunContext() {
        log::info("Shutting down application");
        client::cleanUp();
    };
}
