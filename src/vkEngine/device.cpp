#include "device.h"

namespace polyp {
namespace engine {

namespace {

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
                                const DeviceCreateInfo& info, const VkPhysicalDeviceFeatures* features) {
    VkDevice output = VK_NULL_HANDLE;

    std::vector<VkDeviceQueueCreateInfo> queInfos(info.mQueueInfo.size());
    for (size_t i = 0; i < queInfos.size(); i++) {
        queInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queInfos[i].flags = 0;
        queInfos[i].queueFamilyIndex = info.mQueueInfo[i].mFamilyIndex;
        queInfos[i].queueCount = info.mQueueInfo[i].mPriorities.size();
        queInfos[i].pQueuePriorities = info.mQueueInfo[i].mPriorities.data();
    }

    VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = queInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queInfos.data();
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.enabledExtensionCount = info.mDesiredExtentions.size();
    deviceCreateInfo.ppEnabledExtensionNames = info.mDesiredExtentions.data();
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

Device::Device(Instance::Ptr instance, GpuInfo device) :
               mInfo{}, mDispTable{}, mInstance{ instance }, 
               mGpuInfo{ device }, mHandle{ VK_NULL_HANDLE }
{
    mDispTable = mInstance->dispatchTable();
}

Device::Device(Instance::Ptr instance, GpuInfo device, const DeviceCreateInfo& info) :
               Device(instance, device)
{
    mInfo = info;
}

DispatchTable Device::dispatchTable() const {
    return mDispTable;
}

bool Device::init() {
    
    if (*mGpuInfo == VK_NULL_HANDLE || !mInstance) {
        return false;
    }

    auto deviceExts = getPhysicalDeviceExts(mDispTable.EnumerateDeviceExtensionProperties, *mGpuInfo);
    if (deviceExts.empty() || !checkSupportedExt(deviceExts)) {
        return false;
    }

    if (!checkSupportedQueue()) {
        return false;
    }

    *mHandle = createDevice(mDispTable.CreateDevice, *mGpuInfo, mInfo, nullptr);

    if (!loadVkDevice(*mHandle, mDispTable, deviceExts)) {
        return false;
    }

    initVkDestroyer(mDispTable.DestroyDevice, mHandle, nullptr);

    return *mHandle != VK_NULL_HANDLE;
}

bool Device::checkSupportedExt(const std::vector<VkExtensionProperties>& available) const {
    auto availableItr = available.begin();
    auto desiredItr = mInfo.mDesiredExtentions.begin();

    while (availableItr != available.end() && desiredItr != mInfo.mDesiredExtentions.end()) {
        if (strcmp(availableItr->extensionName, *desiredItr) == 0) {
            desiredItr++;
        }
        availableItr++;
    }

    return desiredItr == mInfo.mDesiredExtentions.end();
}

bool Device::checkSupportedQueue() {

    if (mInfo.mQueueInfo.empty()) {
        return false;
    }

    // Fill QueueCreateInfom.FamilyIndex where its value is UINT32_MAX
    for (size_t i = 0; i < mInfo.mQueueInfo.size(); i++) {
        if (mInfo.mQueueInfo[i].mFamilyIndex != UINT32_MAX) {
            continue;
        }
        QueueCreateInfo& queInfo = mInfo.mQueueInfo[i];
        for (uint32_t index = 0; index < mGpuInfo.queueFamilyCount(); ++index) {
            if ((mGpuInfo.queueCount(index) > queInfo.mPriorities.size()) &&
                 mGpuInfo.queueHasFlags(index, queInfo.mQueueType)) {
                queInfo.mFamilyIndex = index;
                break;
            }
        }
    }

    // Squash queue infos with the same family index
    std::sort(mInfo.mQueueInfo.begin(), mInfo.mQueueInfo.end(), [](auto& lhv, auto& rhv) {
        return lhv.mFamilyIndex < rhv.mFamilyIndex;
        });
    auto queuePrevItr = mInfo.mQueueInfo.begin();
    auto queueCurrItr = mInfo.mQueueInfo.begin() + 1;
    while (queueCurrItr != mInfo.mQueueInfo.end()) {
        if (queueCurrItr->mFamilyIndex == queuePrevItr->mFamilyIndex) {
            queuePrevItr->mQueueType |= queueCurrItr->mQueueType;
            std::copy(queueCurrItr->mPriorities.begin(), queueCurrItr->mPriorities.end(), 
                      std::back_inserter(queuePrevItr->mPriorities));
            queueCurrItr = mInfo.mQueueInfo.erase(queueCurrItr);
            queuePrevItr = queueCurrItr - 1;
        }
        else{
            queuePrevItr++;
            queueCurrItr++;
        }
    }

    std::vector<uint32_t> requestedQueSizes(mGpuInfo.queueFamilyCount(), 0);

    // Checks:
    // - there are no queues with family index UINT32_MAX;
    // - requested queue family index is correct;
    for (size_t i = 0; i < mInfo.mQueueInfo.size(); i++) {
        if (mInfo.mQueueInfo[i].mFamilyIndex == UINT32_MAX ||
            mInfo.mQueueInfo[i].mFamilyIndex >= mGpuInfo.queueCount(i)) {
            return false;
        }
        requestedQueSizes[mInfo.mQueueInfo[i].mFamilyIndex] += mInfo.mQueueInfo[i].mPriorities.size();
    }
 
    // Check that queue family count isn't exceeded
    for (size_t i = 0; i < requestedQueSizes.size(); i++) {
        if (requestedQueSizes[i] > mGpuInfo.queueCount(i)) {
            return false;
        }
    }

    return true;
}

VkDevice const& Device::operator*() const {
    return *mHandle;
}

} // engine
} // polyp
