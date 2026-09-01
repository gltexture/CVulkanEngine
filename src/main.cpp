#include <iostream>

#include "core/engine_context.h"

int main() {
    {
        const auto context = cvulkan::EngineContext({"TestVulkan", {1280, 720}} );
        try {
            context.start();
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return -1;
        }
    }
    return 0;
}