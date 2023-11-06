#include "utils.h"

namespace polyp {
namespace vk {
namespace utils {

VkFence createFence(Device::ConstPtr device, bool signaled)
{
    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    createInfo.pNext = nullptr;
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    CHECKRET(device->vk().CreateFence(**device, &createInfo, nullptr, &fence));
    return fence;
}

BufferResource createBuffer(Device::ConstPtr device, VkBufferUsageFlags usage, uint32_t size, VkMemoryPropertyFlags props) {
    auto vk  = device->vk();
    auto gpu = device->gpu();
    auto dev = device->native();
    
    VkBuffer buffer = VK_NULL_HANDLE;
    VkBufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    createInfo.size = size;
    createInfo.usage = usage;
    CHECKRET(vk.CreateBuffer(dev, &createInfo, nullptr, &buffer));

    VkMemoryRequirements memReqs;
    vk.GetBufferMemoryRequirements(dev, buffer, &memReqs);

    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkMemoryAllocateInfo memAllocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = gpu.memTypeIndex(memReqs, props);
    POLYPASSERTNOTEQUAL(memAllocInfo.memoryTypeIndex, UINT32_MAX);
    CHECKRET(vk.AllocateMemory(dev, &memAllocInfo, nullptr, &memory));

    CHECKRET(vk.BindBufferMemory(dev, buffer, memory, 0));

    return BufferResource{ {memory, device}, {buffer, device} };
};

} // info
} // engine
} // polyp
