#pragma once

#include "core/utility/system.h"

namespace cvulkan::client {
    struct CVulkanClientRuntimeData {
        std::string _title;
        uint32_t _windowWidth;
        uint32_t _windowHeight;
    };

    class CVulkanClientRuntime {
    public:
        explicit CVulkanClientRuntime(const CVulkanClientRuntimeData&& engineData);
        ~CVulkanClientRuntime();

        CLASS_NO_COPY_NO_MOVE(CVulkanClientRuntime)

        void run();

    private:
        const CVulkanClientRuntimeData _engineData;
        bool _initialized = false;
    };

}
