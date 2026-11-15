#include "engine_context.h"

#include "client/client_runtime.h"
#include "utility/logger.inl"
#include "utility/sys_utils.h"

namespace cvulkan {
    CVulkanEngineRunContext::CVulkanEngineRunContext(EngineData&& engineData) : _engineData(std::move(engineData)) {
        {
            sys_utils::utilityInitialize();
        }
        logging::info("Initializing application");
        logging::info("Engine: {}", _engineData._appTitle);
        client::initializeClient(this->_engineData);
        logging::info("Application initialized");
        client::loop();
    }

    CVulkanEngineRunContext::~CVulkanEngineRunContext() {
        logging::info("Shutting down application");
        client::cleanUp();
    };
}
