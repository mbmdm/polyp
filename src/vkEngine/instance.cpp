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

/// Returns physical device properties
[[nodiscard]] auto getPhysicalDeviceInfo(PFN_vkGetPhysicalDeviceProperties pFun,
    VkInstance instance, VkPhysicalDevice device) {
    VkPhysicalDeviceProperties output;
    pFun(device, &output);
    return output;
}

} // anonymous namespace

Instance::Instance() :
    mMajorVersion{ 99 }, mMinorVersion{ 99 }, mPatchVersion{99},
    mAppicationName{ "Polyp application (default)" }, mLibrary{NULL},
    mHandle{ VK_NULL_HANDLE }, mDispTable{}
{
    mExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    mExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
}

Instance::Instance(const char* appName) : Instance() {
    mAppicationName = appName;
}

Instance::Instance(const char* appName, uint32_t major, uint32_t minor, uint32_t patch) : Instance(appName) {
    mMajorVersion = major;
    mMinorVersion = minor;
    mPatchVersion = patch;
}

Instance::Instance(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, 
    const std::vector<const char*>& desiredExt) : Instance(appName, major, minor, patch) {
    mExtensions = desiredExt;
}

std::string Instance::getAppName() const {
    return mAppicationName;
}

std::tuple<uint32_t, uint32_t, uint32_t> Instance::getAppVersion() const {
    return std::make_tuple(mMajorVersion, mMinorVersion, mPatchVersion);
}

std::vector<const char*> Instance::getExtensions() const {
    return mExtensions;
}

DispatchTable Instance::getDispatchTable() const {
    return mDispTable;
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
    std::sort(mExtensions.begin(), mExtensions.end(), comparer);
    std::sort(availableExt.begin(), availableExt.end(), comparer);

    if (!checkSupportedExt(availableExt)) {
        printf("Required vulkan instance extensions aren't supported\n");
        return false;
    }

    *mHandle = createInstance(
        mAppicationName.c_str(), mDispTable.CreateInstance, mExtensions, std::make_tuple(mMajorVersion, mMinorVersion, mPatchVersion));

    loadVkInstance(*mHandle, mDispTable);

    initVkDestroyer(mDispTable.DestroyInstance, mHandle, nullptr);

    return true;
}

bool Instance::checkSupportedExt(const std::vector<VkExtensionProperties>& availableExt) const {
    bool flag = false;
    for (size_t i = 0, j = 0; i < mExtensions.size() && j < availableExt.size() && !flag; j++) {
        auto& lhv = mExtensions[i];
        auto& rhv = availableExt[j].extensionName;
        if (strcmp(lhv, rhv) == 0) {
            flag = i + 1 == mExtensions.size();
            i++;
        }
    }
    return flag;
}

VkInstance const& Instance::operator*() const {
    return *mHandle;
}

} // engine
} // polyp
