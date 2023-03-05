#ifndef UTILS_H
#define UTILS_H

#include "common.h"
#include "device.h"
#include "instance.h"

namespace polyp {
namespace engine {
namespace info {

using GpuInfo = std::tuple<VkPhysicalDevice, VkPhysicalDeviceProperties, VkPhysicalDeviceMemoryProperties>;

/// Returns VkPhysicalDevice and its properties by given device number.
/// Typical usage: start loop to get all the gpu devices until VkPhysicalDevice == VK_NULL_HANDLE
/// 
/// \param instance - polyp::engine::Instance::Ptr.
/// \param num - device index.
[[nodiscard]] GpuInfo getPhysicalGPU(const Instance::Ptr& instance, int num);


/// Returns GPU count in the system
[[nodiscard]] uint32_t getPhysicalGPUCount(const Instance::Ptr& instance);

} // info
} // engine
} // polyp

#endif // UTILS_H
