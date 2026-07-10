#include <iostream>

#include "core/engine_context.h"

int main() {
    const auto context = cvulkan::EngineContext({"TestVulkan", {1280, 720}} );
    context.start();
    return 0;
}
