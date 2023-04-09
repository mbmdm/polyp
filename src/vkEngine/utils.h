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

} // utils
} // engine
} // polyp

#endif // UTILS_H
