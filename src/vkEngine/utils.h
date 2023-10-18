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

/// Creates a new VkFence
VkFence createFence(Device::ConstPtr device, bool signaled = false);

struct BufferResource {
    DESTROYABLE(VkDeviceMemory) memory;
    DESTROYABLE(VkBuffer)       buffer;
};

/// Create a new VkBuffer
BufferResource createBuffer(Device::ConstPtr device, VkBufferUsageFlags usage, uint32_t size, VkMemoryPropertyFlags props);

} // utils
} // engine
} // polyp

#endif // UTILS_H
