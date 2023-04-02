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

/// Creates a new VkFence in signaled state
VkFence createFence(Device::ConstPtr device);

} // utils
} // engine
} // polyp

#endif // UTILS_H
