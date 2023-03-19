#include "instance.h"

namespace polyp {
namespace engine {

namespace {

/// Loads vulkan exported functions and stores them in the dispatch table.
[[nodiscard]] auto loadVkExported(const HMODULE& library, DispatchTable& table) {

#define EXPORTED_VULKAN_FUNCTION( name )                              \
    table.name = (PFN_vk##name)GetProcAddress( library, "vk"#name);   \
    if( table.name == nullptr ) {                                     \
      std::cout << "Could not load exported Vulkan function named: "  \
      "vk"#name << std::endl;                                         \
      return false;                                                   \
    }

#include "dispatch_table.inl"
    return true;
 }

/// Loads vulkan global functions and stores them in the dispatch table.
[[nodiscard]] auto loadVkGlobal(DispatchTable& table) {

#define GLOBAL_LEVEL_VULKAN_FUNCTION( name )                                   \
    table.name = (PFN_vk##name)table.GetInstanceProcAddr(nullptr, "vk"#name);  \
    if( table.name == nullptr ) {                                              \
      std::cout << "Could not load global Vulkan function named: "             \
      "vk"#name << std::endl;                                                  \
      return false;                                                            \
    }

#include "dispatch_table.inl"

    return true;
}

/// Returns available vulkan instance extensions.
[[nodiscard]] auto getInstanceExtensions(PFN_vkEnumerateInstanceExtensionProperties pFun) {
    std::vector<VkExtensionProperties> output{};
    uint32_t count = {};
    CHECKRET(pFun(nullptr, &count, nullptr));
    output.resize(count);
    CHECKRET(pFun(nullptr, &count, output.data()));
    return output;
}

/// Creates vulkan instance.
///
/// \param appName is an application name.
/// \param appVersion - major, minor and patch versions of an application.
/// \param apiVersion - major, minor and patch versions of the required vulkan api.
///
/// \returns VkInstance or VK_NULL_HANDLE when failed.
[[nodiscard]] auto createInstance(
    const char* appName,
    PFN_vkCreateInstance pFun,
    const std::vector<const char*>& desiredExt,
    const std::tuple<uint32_t, uint32_t, uint32_t>& appVersion) {

    auto [majorApp, minorApp, patchApp] = appVersion;

    VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    app_info.pApplicationName = appName;
    app_info.applicationVersion = VK_MAKE_VERSION(majorApp, minorApp, patchApp);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = desiredExt.size();
    create_info.ppEnabledExtensionNames = desiredExt.data();

    VkInstance instance = VK_NULL_HANDLE;
    CHECKRET(pFun(&create_info, nullptr, &instance));
    return instance;
}

/// Loads vulkan instance functions and stores them in the dispatch table
[[nodiscard]] auto loadVkInstance(VkInstance instance, DispatchTable& table) {

#define INSTANCE_LEVEL_VULKAN_FUNCTION( name )                                    \
    table.name = (PFN_vk##name)table.GetInstanceProcAddr( instance, "vk"#name );  \
    if( table.name  == nullptr ) {                                                \
      std::cout << "Could not load Vulkan instance function named: "              \
      "vk"#name << std::endl;                                                     \
      return false;                                                               \
    }

    auto availableExt = getInstanceExtensions(table.EnumerateInstanceExtensionProperties);

#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )                    \
    for (auto& ext : availableExt) {                                                        \
        if (strcmp(ext.extensionName, extension) == 0) {                                    \
            table.name = (PFN_vk##name)table.GetInstanceProcAddr( instance, "vk"#name );    \
            if( table.name == nullptr ) {                                                   \
                std::cout << "Could not load Vulkan instance function named: "              \
                "vk"#name << std::endl; return false;                                       \
            }                                                                               \
         }                                                                                  \
     }

#include "dispatch_table.inl"

     return true;
}

/// Returns info about all the GPUs in system
[[nodiscard]] auto getGpuInfos(Instance::Ptr instance) {
    
    uint32_t count = {};
    CHECKRET(instance->getDispatchTable().EnumeratePhysicalDevices(**instance, &count, nullptr));

    using ReturnT = std::tuple<VkPhysicalDevice, 
                               VkPhysicalDeviceProperties, 
                               VkPhysicalDeviceMemoryProperties,
                               std::vector<VkQueueFamilyProperties>>;

    std::vector<ReturnT>          output(count);
    std::vector<VkPhysicalDevice> devices(count);

    CHECKRET(instance->getDispatchTable().EnumeratePhysicalDevices(**instance, &count, devices.data()));

    for (size_t i = 0; i < count; i++) {
        std::get<0>(output[i]) = devices[i];
        VkPhysicalDeviceProperties       properties;
        VkPhysicalDeviceMemoryProperties memProperties;

        instance->getDispatchTable().GetPhysicalDeviceProperties(devices[i],       &std::get<1>(output[i]));
        instance->getDispatchTable().GetPhysicalDeviceMemoryProperties(devices[i], &std::get<2>(output[i]));
        uint32_t queueFamiliesCount = 0;
        instance->getDispatchTable().GetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamiliesCount, nullptr);
        std::get<3>(output[i]).resize(queueFamiliesCount);
        instance->getDispatchTable().GetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamiliesCount, std::get<3>(output[i]).data());
    }

    return output;
}

} // anonymous namespace

Instance::Instance() :
    mAppicationName{ "Polyp application (default)" }, 
    mInfo{}, mLibrary{ NULL }, mHandle{ VK_NULL_HANDLE }, mDispTable{}
{}

Instance::Instance(const char* appName) : Instance() {
    mAppicationName = appName;
}

Instance::Instance(const char* appName, const InstanceCreateInfo& info) : Instance(appName) {
    mInfo = info;
}

std::string Instance::getAppName() const {
    return mAppicationName;
}

std::tuple<uint32_t, uint32_t, uint32_t> Instance::getAppVersion() const {
    return std::make_tuple(mInfo.mVersion.major, mInfo.mVersion.minor, mInfo.mVersion.patch);
}

std::vector<const char*> Instance::getExtensions() const {
    return mInfo.mDesiredExtentions;
}

DispatchTable Instance::getDispatchTable() const {
    return mDispTable;
}

uint32_t Instance::getSystemGpuCount() const {
    return mGpuInfos.size();
}

GpuInfo Instance::getSystemGpuInfo(int id) const {
    return mGpuInfos.at(id);
}

bool Instance::init() {

    if (mAppicationName.empty() || mLibrary || mDispTable.GetInstanceProcAddr) {
        printf("Something went wrong. Only single init() call can be invoked.\n");
        return false;
    }

    constexpr auto vulkan_lib_name = "vulkan-1.dll";
    *mLibrary = LoadLibraryA(vulkan_lib_name);

    if (!mLibrary) {
        printf("Failed to load %s library\n", vulkan_lib_name);
        return false;
    }

    initVkDestroyer(FreeLibrary, mLibrary);

    if (!loadVkExported(*mLibrary, mDispTable) || !loadVkGlobal(mDispTable)) {
        return false;
    }

    auto availableExt = getInstanceExtensions(mDispTable.EnumerateInstanceExtensionProperties);
    auto comparer = []<typename T>(const T & lhv, const T & rhv) {
        if constexpr (std::is_same_v<T, VkExtensionProperties>)
            return strcmp(lhv.extensionName, rhv.extensionName) < 0;
        else
            return strcmp(lhv, rhv) < 0;
    };
    std::sort(mInfo.mDesiredExtentions.begin(), mInfo.mDesiredExtentions.end(), comparer);
    std::sort(availableExt.begin(), availableExt.end(), comparer);

    if (!checkSupportedExt(availableExt)) {
        printf("Required vulkan instance extensions aren't supported\n");
        return false;
    }

    *mHandle = createInstance(
        mAppicationName.c_str(), mDispTable.CreateInstance, mInfo.mDesiredExtentions, getAppVersion());

    if (!loadVkInstance(*mHandle, mDispTable)) {
        POLYPFATAL("Failed to load vulkan instance functions");
        exit(1);
    }

    initVkDestroyer(mDispTable.DestroyInstance, mHandle, nullptr);

    auto gpus = getGpuInfos(shared_from_this());
    for (size_t i = 0; i < gpus.size(); ++i) {
        GpuInfo gpuInfo;
        gpuInfo.mDevice        = std::get<0>(gpus[i]);
        gpuInfo.mProperties    = std::get<1>(gpus[i]);
        gpuInfo.mMemProperties = std::get<2>(gpus[i]);
        gpuInfo.mQueProperties = std::get<3>(gpus[i]);
        mGpuInfos.push_back(gpuInfo);
    }

    return true;
}

bool Instance::checkSupportedExt(const std::vector<VkExtensionProperties>& availableExt) const {
    bool flag = false;
    auto& extentions = mInfo.mDesiredExtentions; // alias
    for (size_t i = 0, j = 0; i < extentions.size() && j < availableExt.size() && !flag; j++) {
        auto& lhv = extentions[i];
        auto& rhv = availableExt[j].extensionName;
        if (strcmp(lhv, rhv) == 0) {
            flag = i + 1 == extentions.size();
            i++;
        }
    }
    return flag;
}

VkInstance const& Instance::operator*() const {
    return *mHandle;
}

VkDeviceSize GpuInfo::memory() {
    
    VkDeviceSize output = 0;
    std::vector<size_t> targetHeapsIdx;
    for (size_t heapTypeIdx = 0; heapTypeIdx < mMemProperties.memoryTypeCount; ++heapTypeIdx) {
        if (mMemProperties.memoryTypes[heapTypeIdx].propertyFlags &
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            targetHeapsIdx.push_back(mMemProperties.memoryTypes[heapTypeIdx].heapIndex);
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
        output += mMemProperties.memoryHeaps[targetHeapsIdx[i]].size;
    }

    return output;
}

bool GpuInfo::isDiscrete() {
    return mProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

std::string GpuInfo::name() {
    return mProperties.deviceName;
}

uint32_t GpuInfo::queueFamilyCount() {
    return mQueProperties.size();
}

std::vector<bool> GpuInfo::checkSupport(VkQueueFlags flags, uint32_t count) {
    std::vector<bool> output(mQueProperties.size(), false);
    for (size_t i = 0; i < output.size(); i++) {
        if (mQueProperties[i].queueFlags & flags &&
            mQueProperties[i].queueCount >= count) {
            output[i] = true;
        }
    }
    return output;
}

uint32_t GpuInfo::queueCount(int queueIndex) {
    return mQueProperties[queueIndex].queueCount;
}

bool GpuInfo::queueHasFlags(int queueIndex, VkFlags flags) {
    return mQueProperties[queueIndex].queueFlags & flags;
}

VkPhysicalDevice GpuInfo::operator*() {
    return mDevice;
}

} // engine
} // polyp
