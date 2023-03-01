#ifndef DEVICE_H
#define DEVICE_H

#include "dispatch_table.h"
#include "vk_destroyer.h"
#include "instance.h"

namespace polyp {
namespace engine {

class Device final {
public:
    Device(Instance::Ptr instance, VkPhysicalDevice device);
    Device(Instance::Ptr instance, VkPhysicalDevice device, 
           const std::vector<const char*>& desiredExt);

    [[nodiscard]] bool init();

private:
    [[nodiscard]] bool checkSupportedExt(const std::vector<VkExtensionProperties>& available) const;

private:
    Instance::Ptr mInstance;
    DispatchTable mDispTable;
    VkPhysicalDevice mPhysicalDevice;

    std::vector<const char*> mExtensions;

    VkQueueFlags mQueueCapabilities;

};

} // engine
} // polyp

#endif // DEVICE_H