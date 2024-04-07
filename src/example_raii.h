#ifndef EXAMPLERAII_H
#define EXAMPLERAII_H

#include "common.h"

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

#include <polyp_window.h>

namespace polyp {
namespace example {

using namespace tools;

struct ImageResource {
    vk::raii::DeviceMemory memory{ VK_NULL_HANDLE };
    vk::raii::Image        image { VK_NULL_HANDLE };
    vk::raii::ImageView    view  { VK_NULL_HANDLE };
};

struct BufferResource {
    vk::raii::DeviceMemory memory { VK_NULL_HANDLE };
    vk::raii::Image        buffer { VK_NULL_HANDLE };
    vk::raii::ImageView    view   { VK_NULL_HANDLE };
    void* mapping = nullptr;
};

class ExampleBaseRAII : public tools::IRenderer {
protected:

    using FrameBuffers = std::vector<vk::raii::Framebuffer>;
    using Images       = std::vector<vk::Image>;
    using Views        = std::vector<vk::raii::ImageView>;

    vk::raii::Context        mContext        = {};
    vk::raii::Instance       mInstance       = { VK_NULL_HANDLE };
    vk::raii::PhysicalDevice mPhysDevice     = { VK_NULL_HANDLE };
    vk::raii::SurfaceKHR     mSurface        = { VK_NULL_HANDLE };
    vk::raii::Device         mDevice         = { VK_NULL_HANDLE };
    vk::raii::SwapchainKHR   mSwapchain      = { VK_NULL_HANDLE };
    vk::raii::Queue          mQueue          = { VK_NULL_HANDLE };
    vk::raii::CommandPool    mCmdPool        = { VK_NULL_HANDLE };
    vk::raii::CommandBuffer  mCmdBuffer      = { VK_NULL_HANDLE };
    vk::raii::Semaphore      mReadyToPresent = { VK_NULL_HANDLE };
    vk::raii::Fence          mSubmitFence    = { VK_NULL_HANDLE };
    vk::raii::Fence          mAqImageFence   = { VK_NULL_HANDLE };
    vk::raii::RenderPass     mRenderPass     = { VK_NULL_HANDLE };

    VmaAllocator             mVmaAllocator   = { VK_NULL_HANDLE };

    vk::ImageMemoryBarrier   mCurrSwImBarrier;
    uint32_t                 mCurrSwImIndex;

    Images                   mSwapChainImages;
    Views                    mSwapChainVeiews;
    FrameBuffers             mFrameBuffers;

    ImageResource            mDepthStencil;

    void preDraw();
    void postDraw();

public:

    ExampleBaseRAII() {};

    virtual ~ExampleBaseRAII() override { }

    virtual bool onInit(WindowInstance inst, WindowHandle hwnd) override;

    virtual bool onResize() override;

    virtual void draw() override;

    virtual void onShoutDown() override;

    virtual bool isReady() override { return true; }
};

} // example
} // polyp

#endif // EXAMPLERAII_H
