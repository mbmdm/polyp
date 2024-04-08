#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace polyp {
namespace vulkan {

class RHIContext
{
public:
    static RHIContext& get()
    {
        static RHIContext instance;
        return instance;
    }

    RHIContext(const RHIContext&)            = delete;
    RHIContext& operator=(const RHIContext&) = delete;

    const vk::raii::Instance&       instance() const { return mInstance; }
    const vk::raii::PhysicalDevice&      gpu() const { return mGPU; }
    const vk::raii::Device&           device() const { return mDevice; }

    void init(vk::raii::Context&& context, vk::raii::Instance&& instance,  vk::raii::PhysicalDevice&& gpu,
              vk::raii::Device&& device,   vk::raii::SurfaceKHR&& surface, vk::raii::SwapchainKHR&& swapchain)
    {
        mContext   = std::move(context);
        mInstance  = std::move(instance);
        mGPU       = std::move(gpu);
        mDevice    = std::move(device);
        mSwapchain = std::move(swapchain);
    }

private:
    RHIContext() = default;

    vk::raii::Context         mContext = {};
    vk::raii::Instance       mInstance = { VK_NULL_HANDLE };
    vk::raii::PhysicalDevice      mGPU = { VK_NULL_HANDLE };
    vk::raii::Device           mDevice = { VK_NULL_HANDLE };
    vk::raii::SurfaceKHR      mSurface = { VK_NULL_HANDLE };
    vk::raii::SwapchainKHR  mSwapchain = { VK_NULL_HANDLE };
};

}
}
