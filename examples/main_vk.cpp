#include <camera.h>
#include <utils_errors.h>
//#include <shader.h>
#include <device.h>
#include <instance.h>

#include <assert.h>

int main(void) {
    using namespace polyp::engine;
    {
        Instance::Ptr inst = Instance::create("asdf");

        auto deviceCount = inst->getAvailableGpuCount();

        if (deviceCount == 0) {
            return 0;
        }

        auto deviceInfo = inst->getGpuInfo(0);
        auto physDevice = inst->getGpu(0);

        Device device{ inst, physDevice };
        device.init();

    }
}
