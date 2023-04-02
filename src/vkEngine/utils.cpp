#include "utils.h"

namespace polyp {
namespace engine {
namespace utils {
VkFence createFence(Device::ConstPtr device)
{
    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    createInfo.pNext = nullptr;
    CHECKRET(device->vk().CreateFence(**device, &createInfo, nullptr, &fence));
    return fence;
}
} // info
} // engine
} // polyp
