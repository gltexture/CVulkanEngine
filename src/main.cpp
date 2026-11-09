#include <iostream>

#include "core/engine_context.h"
#include "utility/exceptions.h"

int main() {
    {
        std::cout << __cplusplus << '\n';
        const auto context = cvulkan::EngineContext({"TestVulkan", {1280, 720}} );
        try {
            context.start();
        } catch (const cvulkan::exc::CVulkanException& e) {
            std::cerr << "Exception: " << e.what() << '\n';
            std::cerr << "Stacktrace:\n" << e.getStacktrace() << '\n';
            return -1;
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return -2;
        }
    }
    return 0;
}
