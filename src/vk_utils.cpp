#include "vk_utils.h"

namespace polyp {
namespace vulkan {
namespace utils {

bool isDiscrete(const vk::raii::PhysicalDevice& gpu)
{
    ::vk::PhysicalDeviceProperties props = gpu.getProperties();
    return props.deviceType == ::vk::PhysicalDeviceType::eDiscreteGpu;
}

VkDeviceSize getMemorySize(const ::vk::raii::PhysicalDevice& gpu) {

    VkDeviceSize output = 0;

    vk::PhysicalDeviceMemoryProperties memProperties = gpu.getMemoryProperties();

    std::vector<size_t> targetHeapsIdx;
    for (size_t heapTypeIdx = 0; heapTypeIdx < memProperties.memoryTypeCount; ++heapTypeIdx) {
        if (memProperties.memoryTypes[heapTypeIdx].propertyFlags & ::vk::MemoryPropertyFlagBits::eDeviceLocal) {
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

vk::raii::PhysicalDevice getPowerfullGPU(const std::vector<vk::raii::PhysicalDevice>& gpus)
{
    if (gpus.empty())
        return VK_NULL_HANDLE;

    auto output = *gpus.begin();

    for (size_t i = 1; i < gpus.size(); i++) {
        auto gpu = gpus[i];
        if (getMemorySize(gpu) > getMemorySize(output) && isDiscrete(gpu)) {
            output = gpu;
        }
    }

    return output;
}

std::vector<bool> getSupportedQueueFamilies(const vk::raii::PhysicalDevice& gpu, vk::QueueFlags flags, uint32_t count)
{
    auto queFamilyProps = gpu.getQueueFamilyProperties();
    std::vector<bool> output(queFamilyProps.size(), false);

    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;

    for (uint32_t i = 0; i < queFamilyProps.size(); i++)
    {
        if ((queFamilyProps[i].queueFlags & flags) &&
            (queFamilyProps[i].queueCount >= count))
        {
            output[i] = true;
        }
    }

    return output;
}

std::vector<bool> getSupportedQueueFamilies(const vk::raii::PhysicalDevice& gpu, const vk::raii::SurfaceKHR& surface)
{
    auto queFamilyProps = gpu.getQueueFamilyProperties();

    std::vector<bool> output(queFamilyProps.size());
    for (size_t i = 0; i < output.size(); i++)
    {
        output[i] = (gpu.getSurfaceSupportKHR(i, *surface) == VK_TRUE);
    }

    return output;
}

}
}
}
