#include "vk_common.h"

namespace polyp {
namespace vulkan {

std::vector<PhysicalDevice> Instance::enumeratePhysicalDevicesPLP() const
{
    auto gpus = enumeratePhysicalDevices();

    std::vector<PhysicalDevice> output;

    std::transform(gpus.begin(), gpus.end(), std::back_inserter(output), [](auto& gpu) { return gpu; });

    return output;
}

VkDeviceSize PhysicalDevice::getDeviceMemoryPLP() const
{
    VkDeviceSize output = 0;

    vk::PhysicalDeviceMemoryProperties memProperties = getMemoryProperties();

    std::vector<size_t> targetHeapsIdx;
    for (size_t heapTypeIdx = 0; heapTypeIdx < memProperties.memoryTypeCount; ++heapTypeIdx) {
        if (memProperties.memoryTypes[heapTypeIdx].propertyFlags & MemoryPropertyFlagBits::eDeviceLocal) {
            targetHeapsIdx.push_back(memProperties.memoryTypes[heapTypeIdx].heapIndex);
        }
    }

    if (targetHeapsIdx.empty()) {
        return output;
    }

    //remove the same indexes if exist
    std::sort(targetHeapsIdx.begin(), targetHeapsIdx.end(), std::less<size_t>());
    auto currItr = targetHeapsIdx.begin() + 1;
    while (currItr != targetHeapsIdx.end()) {
        if (*currItr == *(currItr - 1)) {
            currItr = targetHeapsIdx.erase(currItr);
        }
        else {
            currItr++;
        }
    }

    for (size_t i = 0; i < targetHeapsIdx.size(); i++) {
        output += memProperties.memoryHeaps[targetHeapsIdx[i]].size;
    }

    return output;
}

bool PhysicalDevice::isDiscretePLP() const
{
    PhysicalDeviceProperties props = getProperties();
    return props.deviceType == PhysicalDeviceType::eDiscreteGpu;
}

uint64_t PhysicalDevice::getPerformanceRatioPLP() const
{
    auto ratio = getDeviceMemoryPLP() >> 16;
    if (isDiscretePLP())
        ratio += 1 << 48;
    
    return ratio;
}

}
}