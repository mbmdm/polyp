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
[[nodiscard]] auto getInstanceExtensions(Instance::ConstPtr instance) {
    std::vector<VkExtensionProperties> output{};
    uint32_t count = {};
    CHECKRET(instance->vk().EnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
    output.resize(count);
    CHECKRET(instance->vk().EnumerateInstanceExtensionProperties(nullptr, &count, output.data()));
    return output;
}

/// Creates vulkan instance.
///
/// \param appName is an application name.
/// \param appVersion - major, minor and patch versions of an application.
/// \param apiVersion - major, minor and patch versions of the required vulkan api.
///
/// \returns VkInstance or VK_NULL_HANDLE when failed.
[[nodiscard]] auto createInstance(Instance::ConstPtr instance) {

    auto [majorApp, minorApp, patchApp] = instance->appVersion();
    auto appName = instance->appName();
    auto ext = instance->extensions();

    VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(majorApp, minorApp, patchApp);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = ext.size();
    createInfo.ppEnabledExtensionNames = ext.data();

#ifdef DEBUG
    const std::vector<const char*> debugLayers{
    "VK_LAYER_KHRONOS_validation"
    };
    createInfo.enabledLayerCount = debugLayers.size();
    createInfo.ppEnabledLayerNames = debugLayers.data();
#endif // DEBUG

    VkInstance instanceOut = VK_NULL_HANDLE;
    CHECKRET(instance->vk().CreateInstance(&createInfo, nullptr, &instanceOut));
    return instanceOut;
}

/// Loads vulkan instance functions and stores them in the dispatch table
[[nodiscard]] auto loadVkInstance(Instance::ConstPtr instance, DispatchTable& table) {

#define INSTANCE_LEVEL_VULKAN_FUNCTION( name )                                              \
    table.name = (PFN_vk##name)table.GetInstanceProcAddr( instance->native(), "vk"#name );  \
    if( table.name  == nullptr ) {                                                          \
      std::cout << "Could not load Vulkan instance function named: "                        \
      "vk"#name << std::endl;                                                               \
      return false;                                                                         \
    }

    auto availableExt = getInstanceExtensions(instance);

#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension )                              \
    for (auto& ext : availableExt) {                                                                  \
        if (strcmp(ext.extensionName, extension) == 0) {                                              \
            table.name = (PFN_vk##name)table.GetInstanceProcAddr( instance->native(), "vk"#name );    \
            if( table.name == nullptr ) {                                                             \
                std::cout << "Could not load Vulkan instance function named: "                        \
                "vk"#name << std::endl; return false;                                                 \
            }                                                                                         \
         }                                                                                            \
     }

#include "dispatch_table.inl"

     return true;
}

/// Returns info about all the GPUs in system
[[nodiscard]] auto getGpuInfos(Instance::ConstPtr instance) {
    
    uint32_t count = {};
    CHECKRET(instance->vk().EnumeratePhysicalDevices(instance->native(), &count, nullptr));

    using ReturnT = std::tuple<VkPhysicalDevice, 
                               VkPhysicalDeviceProperties, 
                               VkPhysicalDeviceMemoryProperties,
                               std::vector<VkQueueFamilyProperties>>;

    std::vector<ReturnT>          output(count);
    std::vector<VkPhysicalDevice> devices(count);

    CHECKRET(instance->vk().EnumeratePhysicalDevices(instance->native(), &count, devices.data()));

    for (size_t i = 0; i < count; i++) {
        std::get<0>(output[i]) = devices[i];
        VkPhysicalDeviceProperties       properties;
        VkPhysicalDeviceMemoryProperties memProperties;

        instance->vk().GetPhysicalDeviceProperties(devices[i],       &std::get<1>(output[i]));
        instance->vk().GetPhysicalDeviceMemoryProperties(devices[i], &std::get<2>(output[i]));
        uint32_t queueFamiliesCount = 0;
        instance->vk().GetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamiliesCount, nullptr);
        std::get<3>(output[i]).resize(queueFamiliesCount);
        instance->vk().GetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamiliesCount, std::get<3>(output[i]).data());
    }

    return output;
}

} // anonymous namespace

Instance::Instance() : 
    mInfo{}, mLibrary{ }, mHandle{ }, mDispTable{}
{
}

Instance::Instance(const InstanceCreateInfo& info) : Instance()
{
    mInfo = info;
}

std::string Instance::appName() const {
    return mInfo.mApplicationName;
}

std::tuple<uint32_t, uint32_t, uint32_t> Instance::appVersion() const {
    return std::make_tuple(mInfo.mMajorVersion, mInfo.mMinorVersion, mInfo.mPatchVersion);
}

std::vector<const char*> Instance::extensions() const {
    return mInfo.mDesiredExtentions;
}

InstanceCreateInfo Instance::info() const {
    return mInfo;
}

DispatchTable Instance::vk() const {
    return mDispTable;
}

uint32_t Instance::gpuCount() const {
    return mGPUs.size();
}

PhysicalGpu Instance::gpu(int id) const {
    return mGPUs.at(id);
}

bool Instance::init() {

    if (mInfo.mApplicationName.empty() || mLibrary || mDispTable.GetInstanceProcAddr) {
        POLYPERROR("Something went wrong on vk instance creation");
        return false;
    }

    *mLibrary = LoadLibraryA(constants::vk::kVkLibraryName);
    if (!mLibrary) {
        POLYPERROR("Failed to load %s library", constants::vk::kVkLibraryName);
        return false;
    }

    mLibrary.setDestroyer(FreeLibrary);

    if (!loadVkExported(*mLibrary, mDispTable) || !loadVkGlobal(mDispTable)) {
        return false;
    }

    auto availableExt = getInstanceExtensions(shared_from_this());
    auto comparer = []<typename T>(const T & lhv, const T & rhv) {
        if constexpr (std::is_same_v<T, VkExtensionProperties>)
            return strcmp(lhv.extensionName, rhv.extensionName) < 0;
        else
            return strcmp(lhv, rhv) < 0;
    };
    std::sort(mInfo.mDesiredExtentions.begin(), mInfo.mDesiredExtentions.end(), comparer);
    std::sort(availableExt.begin(), availableExt.end(), comparer);

    if (!check(availableExt)) {
        POLYPERROR("Required vulkan instance extensions aren't supported");
        return false;
    }

    *mHandle = createInstance(shared_from_this());

    if (!loadVkInstance(shared_from_this(), mDispTable)) {
        POLYPERROR("Failed to load vulkan instance functions");
        return false;
    }

    mHandle.initDestroyer(shared_from_this());

    auto gpus = getGpuInfos(shared_from_this());
    for (size_t i = 0; i < gpus.size(); ++i) {
        PhysicalGpu gpu;
        gpu.mDevice        = std::get<0>(gpus[i]);
        gpu.mProperties    = std::get<1>(gpus[i]);
        gpu.mMemProperties = std::get<2>(gpus[i]);
        gpu.mQueProperties = std::get<3>(gpus[i]);
        mGPUs.push_back(gpu);
    }

    return true;
}

bool Instance::check(const std::vector<VkExtensionProperties>& availableExt) const {
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

VkInstance Instance::native() const {
    return this->operator*();
}

const VkInstance* Instance::pNative() const {
    return &this->operator*();
}

VkDeviceSize PhysicalGpu::memory() {
    
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

bool PhysicalGpu::isDiscrete() {
    return mProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

std::string PhysicalGpu::name() {
    return mProperties.deviceName;
}

uint32_t PhysicalGpu::queueFamilyCount() {
    return mQueProperties.size();
}

std::vector<bool> PhysicalGpu::checkSupport(VkQueueFlags flags, uint32_t count) {
    std::vector<bool> output(mQueProperties.size(), false);
    for (size_t i = 0; i < output.size(); i++) {
        if (mQueProperties[i].queueFlags & flags &&
            mQueProperties[i].queueCount >= count) {
            output[i] = true;
        }
    }
    return output;
}

uint32_t PhysicalGpu::queueCount(int queueIndex) {
    return mQueProperties[queueIndex].queueCount;
}

bool PhysicalGpu::queueHasFlags(int queueIndex, VkFlags flags) {
    return mQueProperties[queueIndex].queueFlags & flags;
}

VkPhysicalDevice PhysicalGpu::operator*() {
    const PhysicalGpu& this_ = *this;
    return *this_;
}

VkPhysicalDevice PhysicalGpu::operator*() const {
    return mDevice;
}

} // engine
} // polyp
