#ifndef DEVICE_H
#define DEVICE_H

#include "common.h"
#include "destroyer.h"
#include "instance.h"

namespace polyp {
namespace engine {

struct QueueCreateInfo {
    uint32_t           mFamilyIndex = UINT32_MAX;
    VkQueueFlags       mQueueType   = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    std::vector<float> mPriorities { 1.0 };
};

struct DeviceCreateInfo {
    std::vector<QueueCreateInfo> mQueueInfo {};
    std::vector<const char*>     mDesiredExtentions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

/// Vulkan engin device.
class Device final : public std::enable_shared_from_this<Device> {
private:
    Device(Instance::Ptr instance, PhysicalGpu device);
    Device(Instance::Ptr instance, PhysicalGpu device, const DeviceCreateInfo& info);

public:
    using Ptr      = std::shared_ptr<Device>;
    using ConstPtr = std::shared_ptr<Device const>;

    Device(const Device&)            = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&)                 = delete;
    Device& operator=(Device&&)      = delete;
    ~Device()                        = default;

    DispatchTable    vk()                                                  const;
    /// Returns associated physical device handle&info.
    PhysicalGpu      gpu()                                                 const;
    /// Return device info
    DeviceCreateInfo info()                                                const;
    /// Returns VkQueue family index and its priority by a given VkQueue handle.
    /// Throws out_of_range exeption when failed.
    std::tuple<uint32_t, double> info(VkQueue queue)                       const;
    /// Returns existed VkQueue according to DeviceCreateInfo.
    /// Throws out_of_range exeption when failed.
    /// \param family - queue family index;
    /// \param index  - index of que in mPriorities field of QueueCreateInfo;
    VkQueue queue(uint32_t family, uint32_t index)                         const;
    /// Returns existed VkQueue.
    /// Throws out_of_range exeption when failed.
    /// \param type      - queue required type (e.g. VK_QUEUE_GRAPHICS_BIT);
    /// \param priority  - queue priority. If priority specified incorrect,
    /// returns closest queue with similar priority;
    VkQueue queue(VkQueueFlags type, double priority = 0.0)                const;
    /// Creates new command buffer.
    /// \param family - queue family index;
    /// \param level  - enum of type VkCommandBufferLevel;
    VkCommandBuffer cmdBuffer(uint32_t family, VkCommandBufferLevel level) const;
    /// Creates new command buffer
    /// \param queue - obtained VkQueue;
    /// \param level - enum of type VkCommandBufferLevel;
    VkCommandBuffer cmdBuffer(VkQueue queue, VkCommandBufferLevel level)   const;

    /// Creates device.
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
    [[nodiscard]] static Ptr create(Args... args) {
        std::shared_ptr<Device> output(new Device(args...));
        if (!output->init()) {
            output.reset();
        }
        return output;
    }

    /// Returns underlying vulkan handle.
    VkDevice const& operator*() const;
    /// Returns underlying vulkan handle.
    VkDevice native()           const;

private:
    [[nodiscard]] bool init();
    [[nodiscard]] bool check(const std::vector<VkExtensionProperties>& available) const;
    [[nodiscard]] bool checkSupportedQueue();

    DeviceCreateInfo                                         mInfo;
    DispatchTable                                            mDispTable;
    Instance::Ptr                                            mInstance;
    PhysicalGpu                                              mPhysicalGpu;
    DESTROYABLE(VkDevice)                                    mHandle;
    /// key - queue family index, value - VkQueues of mPriorities size
    std::unordered_map<uint32_t, std::vector<VkQueue>>       mQueue;
    /// key - queue family index, value - Wrapped VkCommandPool
    std::unordered_map<uint32_t, DESTROYABLE(VkCommandPool)> mCommandPool;
};

} // engine
} // polyp

#endif // DEVICE_H
