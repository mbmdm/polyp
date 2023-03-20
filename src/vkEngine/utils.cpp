#include "utils.h"

namespace polyp {
namespace engine {
namespace utils {

std::vector<VkPresentModeKHR> getSupportedPresentationModes(Instance::Ptr instance, Surface::Ptr surface, 
                                                            const GpuInfo& gpuInfo) {
    uint32_t count = 0;
    CHECKRET(instance->dispatchTable().GetPhysicalDeviceSurfacePresentModesKHR(*gpuInfo, **surface, &count, nullptr));
    std::vector<VkPresentModeKHR> output(count);
    CHECKRET(instance->dispatchTable().GetPhysicalDeviceSurfacePresentModesKHR(*gpuInfo, **surface, &count, output.data()));
    return output;
}

bool checkSupportPresentationMode(Instance::Ptr instance, Surface::Ptr surface,
                                  const GpuInfo& gpuInfo, const VkPresentModeKHR& desired) {
    auto modes = getSupportedPresentationModes(instance, surface, gpuInfo);
    for (size_t i = 0; i < modes.size(); ++i) {
        if (modes[i] == desired) {
            return true;
        }
    }
    return false;
}

VkPresentModeKHR getMostSuitablePresentationMode(Instance::Ptr instance, Surface::Ptr surface, 
                                                 const GpuInfo& gpuInfo) {
    auto modes = getSupportedPresentationModes(instance, surface, gpuInfo);
    std::vector<VkPresentModeKHR> priority{ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
    std::vector<bool> seekRes(priority.size(), false);
    for (size_t i = 0; i < modes.size(); i++) {
        auto seekIt = std::find(priority.begin(), priority.end(), modes[i]);
        if (seekIt != priority.end()) {
            seekRes[seekIt - priority.begin()] = true;
        }
    }
    for (size_t i = 0; i < seekRes.size(); i++) {
        if (seekRes[i]) {
            return priority[i];
        }
    }
    return VK_PRESENT_MODE_MAX_ENUM_KHR;
}

} // info
} // engine
} // polyp
