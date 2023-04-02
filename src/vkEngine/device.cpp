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

/// Creates VkCommandPool
[[nodiscard]] auto createCommandPool(Device::Ptr device, uint32_t queFamilyIdx, 
                                     VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
    VkCommandPool cmdPool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    createInfo.pNext = nullptr;
    createInfo.flags = flags;
    createInfo.queueFamilyIndex = queFamilyIdx;
    CHECKRET(device->vk().CreateCommandPool(**device, &createInfo, nullptr, &cmdPool));
    return cmdPool;
}

[[nodiscard]] auto createCommandBuffer(Device::ConstPtr device, VkCommandPool pool, VkCommandBufferLevel level, uint32_t count) {
    std::vector<VkCommandBuffer> output(count, VK_NULL_HANDLE);
    VkCommandBufferAllocateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    createInfo.pNext = nullptr;
    createInfo.commandPool = pool;
    createInfo.level = level;
    createInfo.commandBufferCount = count;
    CHECKRET(device->vk().AllocateCommandBuffers(**device, &createInfo, output.data()));
    return output;
}

} // anonymous namespace

Device::Device(Instance::Ptr instance, PhysicalGpu device) :
               mInfo{}, mDispTable{}, mInstance{ instance }, 
               mPhysicalGpu{ device }, mHandle{ VK_NULL_HANDLE }
{
    mDispTable = mInstance->vk();
}

Device::Device(Instance::Ptr instance, PhysicalGpu device, const DeviceCreateInfo& info) :
               Device(instance, device)
{
    mInfo = info;
}

DispatchTable Device::vk() const {
    return mDispTable;
}

PhysicalGpu Device::gpu() const {
    return mPhysicalGpu;
}

DeviceCreateInfo Device::info() const {
    return mInfo;
}

std::tuple<uint32_t, double> Device::info(VkQueue que) const {
    uint32_t currFamily      = std::numeric_limits<uint32_t>::max();
    double   currPriority    = std::numeric_limits<double>::max();
    uint32_t currPriorityIdx = std::numeric_limits<uint32_t>::max();

    for (const auto& currQueue : mQueue) {
        for (size_t i = 0; i < currQueue.second.size(); ++i) {
            if (currQueue.second[i] == que) {
                currFamily = currQueue.first;
                currPriorityIdx = i;
            }
        }
    }

    for (size_t i = 0; i < mInfo.mQueueInfo.size(); i++) {
        if (mInfo.mQueueInfo[i].mFamilyIndex == currFamily) {
            currPriority = mInfo.mQueueInfo[i].mPriorities[currPriorityIdx];
        }
    }

    if (currFamily == std::numeric_limits<uint32_t>::max() || 
        currPriorityIdx == std::numeric_limits<uint32_t>::max() ) {
        throw std::out_of_range("Failed to faid appropriate queue family index. Internal error.");
    }

    return std::make_tuple(currFamily, currPriority);
}

VkQueue Device::queue(uint32_t family, uint32_t index) const {
    auto itr = mQueue.find(family);
    if (itr == mQueue.end()) {
        throw std::out_of_range{"Wrong queue family index."};
    }
    return itr->second[index];
}

VkQueue Device::queue(VkQueueFlags type, double priority) const {
    uint32_t currFamily      = std::numeric_limits<uint32_t>::max();
    double   currPriority    = std::numeric_limits<double>::max();
    uint32_t currPriorityIdx = std::numeric_limits<double>::max();

    auto distanceFun = [](auto rhv, auto lhv) {
        return std::abs(lhv - rhv);
    };

    for (size_t i = 0; i < mInfo.mQueueInfo.size(); ++i) {
        auto queInfo = mInfo.mQueueInfo[i];
        if (queInfo.mQueueType & type == type) {
            for (uint32_t j = 0; j < queInfo.mPriorities.size(); j++) {
                auto quePriority = queInfo.mPriorities[j];
                if (quePriority == priority) {
                    return queue(queInfo.mFamilyIndex, j);
                }
                auto currDistance = distanceFun(currPriority, priority);
                auto newDistance  = distanceFun(quePriority, priority);
                if (currDistance > newDistance) {
                    currFamily = queInfo.mFamilyIndex;
                    currPriority = quePriority;
                    currPriorityIdx = j;
                }
            }
        }
    }
    return queue(currFamily, currPriorityIdx);
}

VkCommandBuffer Device::cmdBuffer(uint32_t family, VkCommandBufferLevel level) const {
    auto device = shared_from_this();
    const auto& cmdPoolIt = mCommandPool.find(family);
    if (cmdPoolIt == mCommandPool.end()) {
        return VK_NULL_HANDLE;
    }
    VkCommandPool cmdPool = *cmdPoolIt->second;
    auto cmdBuffers = createCommandBuffer(device, cmdPool, level, 1);
    if (cmdBuffers.empty()) {
        return VK_NULL_HANDLE;
    }
    VkCommandBuffer cmd = *cmdBuffers.begin();
    return cmd;
}

VkCommandBuffer Device::cmdBuffer(VkQueue queue, VkCommandBufferLevel level) const {
    auto [family, _] = info(queue);
    return cmdBuffer(family, level);
}

bool Device::init() {
    
    if (*mPhysicalGpu == VK_NULL_HANDLE || !mInstance) {
        return false;
    }

    auto deviceExts = getPhysicalDeviceExts(mDispTable.EnumerateDeviceExtensionProperties, *mPhysicalGpu);
    if (deviceExts.empty() || !check(deviceExts)) {
        return false;
    }

    if (!checkSupportedQueue()) {
        return false;
    }

    *mHandle = createDevice(mDispTable.CreateDevice, *mPhysicalGpu, mInfo, nullptr);

    if (!loadVkDevice(*mHandle, mDispTable, deviceExts)) {
        return false;
    }

    initVkDestroyer(mDispTable.DestroyDevice, mHandle, nullptr);

    // Getting VkQueues
    for (size_t queFamilyIdx = 0; queFamilyIdx < mInfo.mQueueInfo.size(); ++queFamilyIdx) {
        for (size_t queIdx = 0; queIdx < mInfo.mQueueInfo[queFamilyIdx].mPriorities.size(); queIdx++) {
            VkQueue que = VK_NULL_HANDLE;
            mDispTable.GetDeviceQueue(*mHandle, queFamilyIdx, queIdx, &que);
            mQueue[queFamilyIdx].push_back(que);
        }
        // very-very-very badlooking
        VkCommandPoolWrapper cmdPool{ *mHandle, createCommandPool(shared_from_this(), queFamilyIdx) };
        mCommandPool[queFamilyIdx] = VkDestroyer<VkCommandPoolWrapper>{ cmdPool, nullptr };
        initVkDestroyer(mDispTable.DestroyCommandPool, mCommandPool[queFamilyIdx], nullptr);
    }

    return *mHandle != VK_NULL_HANDLE;
}

bool Device::check(const std::vector<VkExtensionProperties>& available) const {
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
        for (uint32_t index = 0; index < mPhysicalGpu.queueFamilyCount(); ++index) {
            if ((mPhysicalGpu.queueCount(index) > queInfo.mPriorities.size()) &&
                 mPhysicalGpu.queueHasFlags(index, queInfo.mQueueType)) {
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

    std::vector<uint32_t> requestedQueSizes(mPhysicalGpu.queueFamilyCount(), 0);

    // Checks:
    // - there are no queues with family index UINT32_MAX;
    // - requested queue family index is correct;
    for (size_t i = 0; i < mInfo.mQueueInfo.size(); i++) {
        if (mInfo.mQueueInfo[i].mFamilyIndex == UINT32_MAX ||
            mInfo.mQueueInfo[i].mFamilyIndex >= mPhysicalGpu.queueCount(i)) {
            return false;
        }
        requestedQueSizes[mInfo.mQueueInfo[i].mFamilyIndex] += mInfo.mQueueInfo[i].mPriorities.size();
    }
 
    // Check that queue family count isn't exceeded
    for (size_t i = 0; i < requestedQueSizes.size(); i++) {
        if (requestedQueSizes[i] > mPhysicalGpu.queueCount(i)) {
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
