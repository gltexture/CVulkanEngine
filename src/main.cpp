#include <iostream>

#include "core/engine_context.h"
#include "utility/exceptions.h"
#include "utility/logger.h"

using namespace cvulkan;

int main() {
    {
        std::set_terminate([] {
            if (const auto exception = std::current_exception()) {
                try {
                    std::rethrow_exception(exception);
                }
                catch (const std::system_error& e) {
                    std::cerr << "System error: " << e.what() << '\n';
                    std::cerr << "Error code: " << e.code().value() << '\n';
                    std::cerr << "Error category: " << e.code().category().name() << '\n';
                }
                catch (const std::exception& e) {
                    std::cerr << "Unhandled exception: " << e.what() << '\n';
                }
                catch (...) {
                    std::cerr << "Unknown exception\n";
                }
            }
            std::cerr << std::stacktrace::current() << '\n';
            std::abort();
        });
    }
    {
        exc::crash::CCrashReporter::initialize();
    }
    {
        std::cout << "cpp " << __cplusplus << '\n';
        try {
            logging::debug("Destroyed Vulkan descriptor set layout {}");
            CVulkanEngineRunContext({"TestVulkan"} );
        }
        catch (const exc::CVulkanException e) {
            std::cerr << "Exception: " << e.what() << '\n';
            std::cerr << e.getStacktrace();
            throw;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << '\n';
            std::cerr << std::stacktrace::current();
            throw;
        }
    }
    return 0;
}
