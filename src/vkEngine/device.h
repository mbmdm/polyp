#ifndef DEVICE_H
#define DEVICE_H

#include "dispatch_table.h"
#include "vk_destroyer.h"
#include "instance.h"

namespace polyp {
namespace engine {

/// Vulkan engin device.
class Device final {
private:
    Device(Instance::Ptr instance, VkPhysicalDevice device);
    Device(Instance::Ptr instance, VkPhysicalDevice device,
           const std::vector<const char*>& desiredExt);

public:
    using Ptr = std::shared_ptr<Device>;

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(Device&&) = delete;
    ~Device() = default;

    /// Creates device
    /// 
    /// Typical usage:
    /// \code
    ///   create(Instance::Ptr instance, VkPhysicalDevice device, const std::vector<const char*>& desiredExt);
    /// \endcode
    /// 
    /// \param instance - polyp::engine::Instance::Ptr.
    /// \param device - Vulkan physical device.
    /// \param desiredExt - desired extension list to create logical device.
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

    Instance::Ptr mInstance;
    VkPhysicalDevice mPhysicalDevice;
    std::vector<const char*> mExtensions;
    VkQueueFlags mQueueCapabilities;

    DECLARE_VKDESTROYER(VkDevice) mHandle;

    DispatchTable mDispTable;
};

} // engine
} // polyp

#endif // DEVICE_H