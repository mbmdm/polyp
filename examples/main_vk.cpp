#include <camera.h>
#include <utils_errors.h>
//#include <shader.h>
#include <device.h>
#include <instance.h>

int main(void) {

    using namespace polyp::engine;
    {
        Instance::Ptr instance = Instance::create("asdf");
        auto gpu = instance->getGpu(0);
        Device::Ptr device = Device::create(instance, gpu);
    }
}
