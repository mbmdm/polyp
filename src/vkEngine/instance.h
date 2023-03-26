#ifndef INSTANCE_H
#define INSTANCE_H

#include "common.h"
#include "destroyer.h"

namespace polyp {
namespace engine {

extern class Instance;

/// Presents system GPU handle and info
class PhysicalGpu final {
public:
    friend class Instance;

    /// Returns device own memory in bytes
    VkDeviceSize      memory();
    bool              isDiscrete();
    /// Returns device name
    std::string       name();
    /// Returns count of available device queue families
    uint32_t          queueFamilyCount();
    /// Loops over all gpu queue families and returns true if supported
    std::vector<bool> checkSupport(VkQueueFlags flags, uint32_t count);
    /// Returns queue count by specified family index. Throws std::out_of_range
    uint32_t          queueCount(int queueIndex);
    /// Returns true if queue family has specified flag. Throws std::out_of_range
    bool              queueHasFlags(int queueIndex, VkFlags flags);

    VkPhysicalDevice operator*();
    VkPhysicalDevice operator*() const;

private:
    VkPhysicalDevice                     mDevice{};
    VkPhysicalDeviceProperties           mProperties{};
    VkPhysicalDeviceMemoryProperties     mMemProperties{};
    std::vector<VkQueueFamilyProperties> mQueProperties{};
};

struct InstanceCreateInfo {
    InstanceCreateInfo() :
        mVersion{ 99, 99, 99 },
        mDesiredExtentions{ VK_KHR_SURFACE_EXTENSION_NAME, 
                            VK_KHR_WIN32_SURFACE_EXTENSION_NAME }
    {}
    struct {
        uint32_t major;
        uint32_t minor;
        uint32_t patch;
    } mVersion;
    std::vector<const char*> mDesiredExtentions;
};

/// Vulkan engin instance.
class Instance final : public std::enable_shared_from_this<Instance> {
private:
    Instance();
    Instance(const char* appName);
    Instance(const char* appName, const InstanceCreateInfo& info);

public:
    using Ptr = std::shared_ptr<Instance>;

    Instance(const Instance&)            = delete;
    Instance& operator=(const Instance&) = delete;
    Instance(Instance&&)                 = delete;
    Instance& operator=(Instance&&)      = delete;
    ~Instance()                          = default;

    std::string appName()                                 const;
    std::tuple<uint32_t, uint32_t, uint32_t> appVersion() const;
    std::vector<const char*> extensions()                 const;
    DispatchTable dispatchTable()                         const;
    /// Returns GPUs count which presented in the system
    uint32_t gpuCount()                                   const;
    /// Returns GPU info by its id. Throws std::out_of_range
    PhysicalGpu gpu(int id)                               const;

    /// Creates instance
    /// 
    /// Typical usage:
    /// \code
    ///   create(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, 
    ///   const std::vector<const char*>& desiredExt);
    /// \endcode
    ///
    /// \param appName -  an application name;
    /// \param info    - additional instance creation info;
    template<typename ...Args>
    [[nodiscard]] static Ptr create(Args... args) {
        std::shared_ptr<Instance> output(new Instance(args...));
        if (!output->init()) {
            output.reset();
        }
        return output;
    }

    VkInstance const& operator*() const;

private:
    [[nodiscard]] bool init();
    [[nodiscard]] bool checkSupportedExt(const std::vector<VkExtensionProperties>& available) const;

    std::string                     mAppicationName;
    InstanceCreateInfo              mInfo;
    DECLARE_VKDESTROYER(VkLibrary)  mLibrary;
    DECLARE_VKDESTROYER(VkInstance) mHandle;
    DispatchTable                   mDispTable;
    std::vector<PhysicalGpu>        mGPUs;
};

} // engine
} // polyp

#endif // INSTANCE_H
