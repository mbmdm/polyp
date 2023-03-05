#include "utils.h"

namespace polyp {
namespace engine {
namespace info {

GpuInfo getPhysicalGPU(Instance::Ptr instance, int num) {

    GpuInfo output;
    std::get<0>(output) = VK_NULL_HANDLE;

    static std::vector<VkPhysicalDevice> devices;
    static std::vector<VkPhysicalDeviceProperties> commonProperties;
    static std::vector< VkPhysicalDeviceMemoryProperties> memProperties;

    uint32_t count = getPhysicalGPUCount(instance);

    if (devices.empty() != count) {

        devices.resize(count);
        commonProperties.resize(count);
        memProperties.resize(count);

        CHECKRET(instance->getDispatchTable().EnumeratePhysicalDevices(**instance, &count, devices.data()));

        for (size_t i = 0; i < count; i++) {
            instance->getDispatchTable().GetPhysicalDeviceProperties(devices[i], commonProperties.data() + i);
            instance->getDispatchTable().GetPhysicalDeviceMemoryProperties(devices[i], memProperties.data() + i);
        }
    }
    if (num < devices.size()) {
        std::get<0>(output) = devices.at(num);
        std::get<1>(output) = commonProperties.at(num);
        std::get<2>(output) = memProperties.at(num);
    }
    return output;
}

uint32_t getPhysicalGPUCount(Instance::Ptr instance) {
    static uint32_t count = UINT32_MAX;

    if (count == UINT32_MAX) {
        CHECKRET(instance->getDispatchTable().EnumeratePhysicalDevices(**instance, &count, nullptr));
    }
    return count;
}



} // info
} // engine
} // polyp
