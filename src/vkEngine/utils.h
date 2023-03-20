#ifndef UTILS_H
#define UTILS_H

#include "common.h"
#include "device.h"
#include "surface.h"
#include "instance.h"
#include "platforms.h"

namespace polyp {
namespace engine {
namespace utils {

/// Returns list of supported presentation modes
std::vector<VkPresentModeKHR> getSupportedPresentationModes(Instance::Ptr instance, Surface::Ptr surface,
                                                            const GpuInfo& gpuInfo);

/// Returns true is desired presentation mode is supported
bool checkSupportPresentationMode(Instance::Ptr instance, Surface::Ptr surface, 
                                  const GpuInfo& gpuInfo, const VkPresentModeKHR& desired);

/// Returns the best available presentation mode. Returns VK_PRESENT_MODE_MAX_ENUM_KHR if failed
VkPresentModeKHR getMostSuitablePresentationMode(Instance::Ptr instance, Surface::Ptr surface, 
                                                 const GpuInfo& gpuInfo);

} // utils
} // engine
} // polyp

#endif // UTILS_H
