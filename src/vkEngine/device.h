#ifndef DEVICE_H
#define DEVICE_H

#include "common.h"
#include "destroyer.h"
#include "instance.h"

namespace polyp {
namespace engine {

struct QueueCreateInfo {
    QueueCreateInfo() :
        mFamilyIndex{ UINT32_MAX },
        mQueueType{ VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT },
        mPriorities { {1.} }
    {}
    uint32_t           mFamilyIndex;
    VkQueueFlags       mQueueType;
    std::vector<float> mPriorities;
};

struct DeviceCreateInfo {
    DeviceCreateInfo() :
        mQueueInfo{ {} },
        mDesiredExtentions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME }
    {}
    std::vector<QueueCreateInfo> mQueueInfo;
    std::vector<const char*> mDesiredExtentions;
};

/// Vulkan engin device.
class Device final {
private:
    Device(Instance::Ptr instance, VkPhysicalDevice device);
    Device(Instance::Ptr instance, VkPhysicalDevice device, const DeviceCreateInfo& info);

public:
    using Ptr = std::shared_ptr<Device>;

    Device(const Device&)            = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&)                 = delete;
    Device& operator=(Device&&)      = delete;
    ~Device()                        = default;

    DispatchTable getDispatchTable() const;

    /// Creates device
    /// 
    /// Typical usage:
    /// \code
    ///   create(Instance::Ptr instance, VkPhysicalDevice device, const DeviceCreateInfo& info);
    /// \endcode
    /// 
    /// \param instance - polyp::engine::Instance::Ptr;
    /// \param device   - vulkan physical device;
    /// \param info     - additional device creation info;
    template<typename ...Args>
    static Ptr create(Args... args) {
        std::shared_ptr<Device> output(new Device(args...));
        if (!output->init()) {
            output.reset();
        }
        return output;
    }

    VkDevice const& operator*() const;

private:
    [[nodiscard]] bool init();
    [[nodiscard]] bool checkSupportedExt(const std::vector<VkExtensionProperties>& available) const;
    [[nodiscard]] bool checkSupportedQueue(const std::vector<VkQueueFamilyProperties>& available);

    DeviceCreateInfo              mInfo;
    DispatchTable                 mDispTable;
    Instance::Ptr                 mInstance;
    VkPhysicalDevice              mPhysicalDevice;
    DECLARE_VKDESTROYER(VkDevice) mHandle;
};

} // engine
} // polyp

#endif // DEVICE_H
