#ifndef INSTANCE_H
#define INSTANCE_H

#include "common.h"
#include "destroyer.h"

namespace polyp {
namespace engine {

class Instance;

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
    uint32_t mMajorVersion = constants::kMajorVersion;
    uint32_t mMinorVersion = constants::kMinorVersion;
    uint32_t mPatchVersion = constants::kPatchVersion;
    std::vector<const char*> mDesiredExtentions{ VK_KHR_SURFACE_EXTENSION_NAME, 
                                                 VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    std::string              mApplicationName = constants::kInternalApplicationName;
};

/// Vulkan engin instance.
class Instance final : public std::enable_shared_from_this<Instance> {
private:
    Instance();
    Instance(const InstanceCreateInfo& info);

public:
    using Ptr      = std::shared_ptr<Instance>;
    using ConstPtr = std::shared_ptr<Instance const>;

    Instance(const Instance&)            = delete;
    Instance& operator=(const Instance&) = delete;
    Instance(Instance&&)                 = delete;
    Instance& operator=(Instance&&)      = delete;
    ~Instance()                          = default;

    /// Returns instance assiciated application name.
    std::string                              appName()    const;
    /// Returns instance assiciated application version.
    std::tuple<uint32_t, uint32_t, uint32_t> appVersion() const;
    /// Returns acquired vulkan extentions.
    std::vector<const char*>                 extensions() const;
    /// Returns instance create info
    InstanceCreateInfo                       info()       const;
    /// Returns vulkan dispatch table.
    DispatchTable vk()                                    const;
    /// Returns available GPUs count in the system.
    uint32_t      gpuCount()                              const;
    /// Returns GPU info by its index.
    /// Throws std::out_of_range.
    PhysicalGpu   gpu(int id)                             const;

    /// Creates instance
    /// 
    /// Typical usage:
    /// \code
    ///   create(const char* appName, uint32_t major, uint32_t minor, uint32_t patch, 
    ///   const std::vector<const char*>& desiredExt);
    /// \endcode
    ///
    /// \param info - instance creation info;
    template<typename ...Args>
    [[nodiscard]] static Ptr create(Args... args) {
        std::shared_ptr<Instance> output(new Instance(args...));
        if (!output->init()) {
            output.reset();
        }
        return output;
    }

    /// Returns underlying vulkan handle.
    VkInstance const& operator*() const;
    /// Returns underlying vulkan handle.
    VkInstance raw()              const;

private:
    [[nodiscard]] bool init();
    [[nodiscard]] bool check(const std::vector<VkExtensionProperties>& available) const;

    InstanceCreateInfo              mInfo;
    DECLARE_VKDESTROYER(VkLibrary)  mLibrary;
    DECLARE_VKDESTROYER(VkInstance) mHandle;
    DispatchTable                   mDispTable;
    std::vector<PhysicalGpu>        mGPUs;
};

} // engine
} // polyp

#endif // INSTANCE_H
