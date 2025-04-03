#pragma once

#include "vk_common.h"

#include <string>
#include <map>

namespace polyp {
namespace vulkan {

class RHIContext
{
public:
    struct CreateInfo
    {
        struct Application
        {
            std::string    name;
            uint32_t    version;
        } app;

        enum class GPU
        {
            Powerful
        } gpu;

        struct Surface
        {
            HWND        handle = NULL;
            HINSTANCE instance = NULL;
        } win;

        struct Queue
        {
            uint32_t          count;
            QueueFlags        flags;
            bool       isWSIRequred;
        };

        struct Device
        {
            std::vector<Queue>       queues;
            PhysicalDeviceFeatures features;
        } device;

        struct SwapChain
        {
            uint32_t count; // image count
        } swapchain;
    };

    static RHIContext& get()
    {
        static RHIContext instance;
        return instance;
    }

    RHIContext(const RHIContext&)            = delete;
    RHIContext& operator=(const RHIContext&) = delete;

    const Instance&       instance() const { return mInstance; }
    const PhysicalDevice&      gpu() const { return mGPU; }
    const SurfaceKHR&      surface() const { return mSurface; }
    const Device&           device() const { return mDevice; }
    const Swapchain&     swapchain() const { return mSwapchain; }

    uint32_t queueFamily(QueueFlags flags) const;

    void init(const CreateInfo& info);
    void init(const CreateInfo::Application& info);
    void init(const CreateInfo::GPU info);
    void init(const CreateInfo::Surface& info);
    void init(const CreateInfo::Device& info);
    void init(const CreateInfo::SwapChain& info);

    void clear()
    {
        mSwapchain.clear();
        mDevice.clear();
        mGPU.clear();
        mInstance.clear();
        mContext = {};
    }

    bool ready() const noexcept
    {
        return (*mInstance  != VK_NULL_HANDLE &&
                *mGPU       != VK_NULL_HANDLE &&
                *mDevice    != VK_NULL_HANDLE &&
                *mSwapchain != VK_NULL_HANDLE);
    }

    void onResize() { init(mCreateInfo.swapchain); }

private:
    RHIContext() = default;

    Context         mContext = {};
    Instance       mInstance = { VK_NULL_HANDLE };
    PhysicalDevice      mGPU = { VK_NULL_HANDLE };
    Device           mDevice = { VK_NULL_HANDLE };
    SurfaceKHR      mSurface = { VK_NULL_HANDLE };
    Swapchain     mSwapchain = { VK_NULL_HANDLE };

    std::map<QueueFlags, uint32_t> mQueueFamilies = {};
    CreateInfo                     mCreateInfo    = {};
};

}
}
