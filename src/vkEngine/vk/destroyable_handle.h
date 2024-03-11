#ifndef HANDLE_DESTROYER_H
#define HANDLE_DESTROYER_H

#include "dispatch_table.h"

#include <polyp_logs.h>

#include <memory>

namespace polyp {
namespace vk {

class Device;
class Instance;

template<typename VkHandle>
class DestroyableHandle final
{
public:
    explicit DestroyableHandle() :
        mInternal{ VK_NULL_HANDLE }
    { }

    explicit DestroyableHandle(VkHandle handle) :
        mInternal{ handle }
    { }

    DestroyableHandle(VkHandle handle, std::shared_ptr<Device const> device):
        mInternal{ handle },
        mDevice{ device }
    { }

    DestroyableHandle(VkHandle handle, std::shared_ptr<Instance const> instance) :
        mInternal{ handle },
        mInstance{ instance }
    { }

    DestroyableHandle(DestroyableHandle&& other) noexcept :
        mInternal{ other.mInternal },
        mDevice{ other.mDevice },
        mInstance{ other.mInstance }
    {
        other.mInternal = VK_NULL_HANDLE;
        other.mDevice.reset();
        other.mInstance.reset();
    }

    void Inst(std::shared_ptr<Instance> instance)
    {
        if (mInstance.get())
        {
            POLYPERROR("Overriding existing instance");
            return;
        }
        mInstance = instance;
    }
    std::shared_ptr<Device> Inst() { return mInstance.lock(); };

    void Dev(std::shared_ptr<Device> device)
    { 
        if (mDevice.lock()) 
        {
            POLYPERROR("Overriding existing device");
            return;
        }
        mDevice = device; 
    }
    std::shared_ptr<Device> Dev() { return mDevice.lock(); };

    DestroyableHandle(const DestroyableHandle&)            = delete;
    DestroyableHandle& operator=(DestroyableHandle const&) = delete;

    DestroyableHandle<VkHandle>& operator=(DestroyableHandle<VkHandle>&& other) noexcept
    {
        if (mInternal == other.mInternal)
            return *this;

        auto handle = mInternal;
        auto device = mDevice;

        mInternal = other.mInternal;
        mDevice   = other.mDevice;

        other.mInternal = handle;
        other.mDevice   = device;

        return *this;
    }

    VkHandle&       operator*()       noexcept { return mInternal; }
    VkHandle*       operator&()       noexcept { return &mInternal; }
    const VkHandle& operator*() const noexcept { return mInternal; }
    const VkHandle* operator&() const noexcept { return &mInternal; }

    bool operator!() const noexcept { return mInternal == VK_NULL_HANDLE; }
    operator bool()  const noexcept { return mInternal != VK_NULL_HANDLE; }

    ~DestroyableHandle() { Destroy(); }

private:
    void Destroy();

    VkHandle                      mInternal;
    std::weak_ptr<Device const>   mDevice;
    std::weak_ptr<Instance const> mInstance;
};

} // engine
} // polyp

#endif // HANDLE_DESTROYER_H