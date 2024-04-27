#include "vk_utils.h"

//using namespace vk::raii;

namespace polyp {
namespace vulkan {
namespace utils {

//bool isDiscrete(const PhysicalDevice& gpu)
//{
//    vk::PhysicalDeviceProperties props = gpu.getProperties();
//    return props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
//}

//VkDeviceSize getMemorySize(const PhysicalDevice& gpu) {
//
//    VkDeviceSize output = 0;
//
//    vk::PhysicalDeviceMemoryProperties memProperties = gpu.getMemoryProperties();
//
//    std::vector<size_t> targetHeapsIdx;
//    for (size_t heapTypeIdx = 0; heapTypeIdx < memProperties.memoryTypeCount; ++heapTypeIdx) {
//        if (memProperties.memoryTypes[heapTypeIdx].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) {
//            targetHeapsIdx.push_back(memProperties.memoryTypes[heapTypeIdx].heapIndex);
//        }
//    }
//
//    if (targetHeapsIdx.empty()) {
//        return output;
//    }
//
//    //remove the same indexes if exist
//    std::sort(targetHeapsIdx.begin(), targetHeapsIdx.end(), std::less<size_t>());
//    auto currItr = targetHeapsIdx.begin() + 1;
//    while (currItr != targetHeapsIdx.end()) {
//        if (*currItr == *(currItr - 1)) {
//            currItr = targetHeapsIdx.erase(currItr);
//        }
//        else {
//            currItr++;
//        }
//    }
//
//    for (size_t i = 0; i < targetHeapsIdx.size(); i++) {
//        output += memProperties.memoryHeaps[targetHeapsIdx[i]].size;
//    }
//
//    return output;
//}

PhysicalDevice getPowerfullGPU(const std::vector<PhysicalDevice>& gpus)
{
    if (gpus.empty())
        return VK_NULL_HANDLE;

    auto output = *gpus.begin();

    for (size_t i = 1; i < gpus.size(); i++) {
        auto gpu = gpus[i];
        if (gpu.getDeviceMemoryPLP() > output.getDeviceMemoryPLP() && gpu.isDiscretePLP()) {
            output = gpu;
        }
    }

    return output;
}

//bool checkSupport(const vk::raii::PhysicalDevice& gpu, vk::Format format)
//{
//    auto props = gpu.getFormatProperties(format);
//    if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
//        return true;
//
//    return false;
//}

//vk::Format getDepthFormat(const PhysicalDevice& gpu)
//{
//    std::vector<vk::Format> dsDesiredFormats = {
//        vk::Format::eD32SfloatS8Uint,
//        vk::Format::eD32Sfloat,
//        vk::Format::eD24UnormS8Uint,
//        vk::Format::eD16UnormS8Uint,
//        vk::Format::eD16Unorm
//    };
//
//    auto depthFormat = vk::Format::eUndefined;
//
//    for (const auto& format : dsDesiredFormats) {
//        auto props = gpu.getFormatProperties(format);
//        if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
//            depthFormat = format;
//            break;
//        }
//    }
//
//    return depthFormat;
//}

bool checkSupport(const PhysicalDevice& device, const SurfaceKHR& surface, vk::PresentModeKHR mode)
{
    auto capabilities = device.getSurfaceCapabilitiesKHR(*surface);
    if (capabilities.currentExtent.width  == UINT32_MAX ||
        capabilities.currentExtent.height == UINT32_MAX) {
        return false;
    }
    else if (capabilities.currentExtent.width == 0 ||
        capabilities.currentExtent.height == 0) {
        return false;
    }

    bool result = false;
    auto presentModes = device.getSurfacePresentModesKHR(*surface);

    for (size_t i = 0; i < presentModes.size(); ++i) {
        if (presentModes[i] == mode) {
            result = true;
            break;
        }
    }
    if (!result && !presentModes.empty())
    {
        mode = presentModes[0];
        result = true;
    }

    return result;
}

}
}
}
