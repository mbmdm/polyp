#include "device.h"

#include <exception>

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
                                const std::vector<char const*>& extensions, uint32_t queueIndex,
                                const std::vector<float>& queuePriorities, const VkPhysicalDeviceFeatures* features) {
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
    deviceCreateInfo.pEnabledFeatures = features;

    CHECKRET(pFun(device, &deviceCreateInfo, nullptr, &output));

    return output;
}

/// Loads vulkan device functions and stores them in the dispatch table.
[[nodiscard]] auto loadVkDevice(VkDevice device, DispatchTable& table,
                                const std::vector<VkExtensionProperties>& availableExt) {

#define DEVICE_LEVEL_VULKAN_FUNCTION( name )                                 \
    table.name = (PFN_vk##name)table.GetDeviceProcAddr( device, "vk"#name ); \
    if( table.name  == nullptr ) {                                           \
      std::cout << "Could not load Vulkan function named: "                  \
        "vk"#name << std::endl;                                              \
      return false;                                                          \
    }

#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )               \
    for (auto& ext : availableExt) {                                                 \
        if (strcmp(ext.extensionName, extension) == 0) {                             \
            table.name = (PFN_vk##name)table.GetDeviceProcAddr( device, "vk"#name ); \
            if( table.name == nullptr ) {                                            \
                std::cout << "Could not load Vulkan function named: "                \
                "vk"#name << std::endl;                                              \
            }                                                                        \
         }                                                                           \
     }

#include "dispatch_table.inl"

    return true;
}

} // anonymous namespace

Device::Device(Instance::Ptr instance, VkPhysicalDevice device) : 
               mInstance{ instance }, mDispTable{}, 
               mPhysicalDevice{ device }, mHandle{ VK_NULL_HANDLE } 
{
    mDispTable = mInstance->getDispatchTable();
    mExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    mQueueCapabilities = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
}

Device::Device(Instance::Ptr instance, VkPhysicalDevice device, 
               const std::vector<const char*>& desiredExt) : Device(instance, device) 
{
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

    *mHandle = createDevice(mDispTable.CreateDevice, mPhysicalDevice, mExtensions, 
                            queFamilyIndex, quePriorities, nullptr);

    if (!loadVkDevice(*mHandle, mDispTable, deviceExts)) {
        return false;
    }

    initVkDestroyer(mDispTable.DestroyDevice, mHandle, nullptr);

    mInstance.reset(); //clear instance due to we dont neet it any more here

    return *mHandle != VK_NULL_HANDLE;
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

VkDevice const& Device::operator*() const {
    return *mHandle;
}

} // engine
} // polyp
