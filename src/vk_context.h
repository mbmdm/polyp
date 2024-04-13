#pragma once

#include "platforms.h"

#include <vulkan/vulkan_raii.hpp>

#include <string>

namespace polyp {
namespace vulkan {

class RHIContext
{
public:
    struct CreateInfo
    {
        struct Application
        {
            std::string name;
            uint32_t    version;
        } app;

        struct Surface
        {
            WindowInstance instance;
            WindowHandle   handle;
        } win;

        struct Queue
        {
            uint32_t       count;
            vk::QueueFlags flags;
        } queue;

        struct Device
        {
            vk::PhysicalDeviceFeatures features;
        } device;
    };

    static RHIContext& get()
    {
        static RHIContext instance;
        return instance;
    }

    RHIContext(const RHIContext&)            = delete;
    RHIContext& operator=(const RHIContext&) = delete;

    const vk::raii::Instance&       instance() const { return mInstance; }
    const vk::raii::PhysicalDevice&      gpu() const { return mGPU; }
    const vk::raii::SurfaceKHR&      surface() const { return mSurface; }
    const vk::raii::Device&           device() const { return mDevice; }

    vk::raii::Queue queue(uint32_t index) { return mDevice.getQueue(mQueueFamilyIndex, index); }


    void init(const CreateInfo::Application& info);
    void init(const CreateInfo::Application& info);
    void init(const CreateInfo& info);

    void clear()
    {
        mSwapchain.clear();
        mDevice.clear();
        mGPU.clear();
        mInstance.clear();
        mContext = {};
    }

    bool ready() const 
    {
        return (*mInstance != VK_NULL_HANDLE && 
                *mGPU != VK_NULL_HANDLE && 
                *mDevice != VK_NULL_HANDLE && 
                *mSwapchain != VK_NULL_HANDLE);
    }

private:
    RHIContext() = default;

    vk::raii::Context         mContext = {};
    vk::raii::Instance       mInstance = { VK_NULL_HANDLE };
    vk::raii::PhysicalDevice      mGPU = { VK_NULL_HANDLE };
    vk::raii::Device           mDevice = { VK_NULL_HANDLE };
    vk::raii::SurfaceKHR      mSurface = { VK_NULL_HANDLE };
    vk::raii::SwapchainKHR  mSwapchain = { VK_NULL_HANDLE };

    uint32_t mQueueFamilyIndex         = {};
};

}
}
