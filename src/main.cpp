#include <iostream>

#include "client/client_runtime.h"
#include "core/utility/exceptions.h"
#include "core/utility/logging.h"

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
            client::CVulkanClientRuntime({"TestVulkan", 1280, 720});
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
