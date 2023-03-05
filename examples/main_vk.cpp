#include <camera.h>
#include <utils_errors.h>
#include <device.h>
#include <instance.h>
#include <utils.h>
#include <window_surface.h>

int main() {

    using namespace polyp::engine;
    using namespace polyp::tools;
    {
        Instance::Ptr instance = Instance::create("asdf");
        auto [gpu, info, memInfo] = info::getPhysicalGPU(instance, 0);
        Device::Ptr device = Device::create(instance, gpu);
        
        auto vk = device->getDispatchTable();

        for (size_t i = 0; i < 10; i++) {
            auto [_, info_, memInfo_] = info::getPhysicalGPU(instance, i);
            if (_ == VK_NULL_HANDLE) {
                break;;
            }

            uint64_t mem = 0;
            for (size_t j = 0; j < memInfo_.memoryHeapCount; j++) {
                mem += memInfo_.memoryHeaps[j].size;
            }
            printf("ANISLOG: found gpu: %s with mem %lu\n", info_.deviceName, mem / (1024 * 1024 * 1024));
        }

        WindowSurface surface{ "123123123 hello",0,0, 800, 800, nullptr };
        surface.run();

    }
}
