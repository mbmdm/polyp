#ifndef INSTANCE_H
#define INSTANCE_H

#include "common.h"
#include "destroyer.h"

namespace polyp {
namespace engine {

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
class Instance final {
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

    std::string getAppName()                                 const;
    std::tuple<uint32_t, uint32_t, uint32_t> getAppVersion() const;
    std::vector<const char*> getExtensions()                 const;
    DispatchTable getDispatchTable()                         const;

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
    static Ptr create(Args... args) {
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
};

} // engine
} // polyp

#endif // INSTANCE_H
