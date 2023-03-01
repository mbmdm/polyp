#include "device.h"

#include <exception>
//TODO: need some special macro for assertion and put it in the common.h (like RDCASSERT)
#include <assert.h>

namespace polyp {
namespace engine {

namespace {

constexpr uint32_t kQueueCount = 1;
constexpr float kQueueDefaultPriority = 1.;

/// Returns supported device extensions in a sorted order
auto getPhysicalDeviceExts(PFN_vkEnumerateDeviceExtensionProperties pFun, VkPhysicalDevice device) {
    std::vector<VkExtensionProperties> exts;
    uint32_t count = 0;
    CHECKRET(pFun(device, nullptr, &count, nullptr));
    exts.resize(count);
    CHECKRET(pFun(device, nullptr, &count, exts.data()));
    std::sort(exts.begin(), exts.end(), [](auto& lhv, auto& rhv) {
        return strcmp(lhv.extensionName, rhv.extensionName) < 0;
        });
    return exts;
}

/// Returns supported device features
[[nodiscard]] auto getPhysicalDeviceFeatures(PFN_vkGetPhysicalDeviceFeatures pFun, VkPhysicalDevice device) {
    VkPhysicalDeviceFeatures features;
    pFun(device, &features);
    return features;
}

/// Returns supported device properties
[[nodiscard]] auto getPhysicalDeviceProperties(PFN_vkGetPhysicalDeviceProperties pFun, VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    pFun(device, &properties);
    return properties;
}

[[nodiscard]] auto getQueueFamiliesProperties(PFN_vkGetPhysicalDeviceQueueFamilyProperties pFun, VkPhysicalDevice device) {
    std::vector<VkQueueFamilyProperties> output;
    uint32_t queue_families_count = 0;
    pFun(device, &queue_families_count, nullptr);
    output.resize(queue_families_count);
    pFun(device, &queue_families_count, output.data());
    return output;
}

[[nodiscard]] auto createDevice(PFN_vkCreateDevice pFun, VkPhysicalDevice device, 
                                std::vector<char const*> const& extensions, uint32_t queueIndex, 
                                const std::vector<float> queuePriorities, const VkPhysicalDeviceFeatures& features) {
    VkDevice output = VK_NULL_HANDLE;

    VkDeviceQueueCreateInfo queCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queCreateInfo.flags = 0;
    queCreateInfo.queueFamilyIndex = queueIndex;
    queCreateInfo.queueCount = queuePriorities.size();
    queCreateInfo.pQueuePriorities = queuePriorities.data();

    VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queCreateInfo;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.enabledExtensionCount = extensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = extensions.data();
    deviceCreateInfo.pEnabledFeatures = &features;

    CHECKRET(pFun(device, &deviceCreateInfo, nullptr, &output));

    return output;
}

} // anonymous namespace

Device::Device(Instance::Ptr instance, VkPhysicalDevice device) : 
               mInstance{ instance }, mDispTable{}, mPhysicalDevice{ device }  {
    mDispTable = mInstance->getDispatchTable();
    mExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    mQueueCapabilities = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
}

Device::Device(Instance::Ptr instance, VkPhysicalDevice device, 
               const std::vector<const char*>& desiredExt) : Device(instance, device) {
    mExtensions = desiredExt;
}

bool Device::init() {
    
    if (!mPhysicalDevice) {
        return false;
    }

    auto deviceExts = getPhysicalDeviceExts(mDispTable.EnumerateDeviceExtensionProperties, mPhysicalDevice);
    if (deviceExts.empty()) {
        return false;
    }

    //TODO: maybe need some logic based on checking features and properties
    auto features = getPhysicalDeviceFeatures(mDispTable.GetPhysicalDeviceFeatures, mPhysicalDevice);
    auto properties = getPhysicalDeviceProperties(mDispTable.GetPhysicalDeviceProperties, mPhysicalDevice);

    if (!checkSupportedExt(deviceExts)) {
        return false;
    }

    auto queFamiliesProperties = getQueueFamiliesProperties(mDispTable.GetPhysicalDeviceQueueFamilyProperties, mPhysicalDevice);
    auto queFamilyIndex = UINT32_MAX;
    for (uint32_t index = 0; index < static_cast<uint32_t>(queFamiliesProperties.size()); ++index) {
        if ((queFamiliesProperties[index].queueCount > 0) &&
            (queFamiliesProperties[index].queueFlags & mQueueCapabilities)) {
            queFamilyIndex = index;
            break;
        }
    }
    if (queFamilyIndex == UINT32_MAX) {
        return false;
    }

    std::vector<float> quePriorities(kQueueCount, kQueueDefaultPriority);

    //TODO: turn off unused fatures
    auto logicalDevice = createDevice(mDispTable.CreateDevice, mPhysicalDevice, 
                                      mExtensions, queFamilyIndex, quePriorities, features);

    assert(!"Need to make logical device desctroible");

    return true;
}

bool Device::checkSupportedExt(const std::vector<VkExtensionProperties>& available) const {
    auto availableItr = available.begin();
    auto desiredItr = mExtensions.begin();

    while (availableItr != available.end() && desiredItr != mExtensions.end()) {
        if (strcmp(availableItr->extensionName, *desiredItr) == 0) {
            desiredItr++;
        }
        availableItr++;
    }

    return desiredItr == mExtensions.end();
}

} // engine
} // polyp
