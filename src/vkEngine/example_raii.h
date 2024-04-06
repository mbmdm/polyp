#ifndef EXAMPLERAII_H
#define EXAMPLERAII_H

#include "common.h"

#include <vulkan/vulkan_raii.hpp>

#include <polyp_window.h>

namespace polyp {
namespace example {

using namespace tools;

struct ImageResourceRAII {
    vk::raii::DeviceMemory memory{ VK_NULL_HANDLE };
    vk::raii::Image        image { VK_NULL_HANDLE };
    vk::raii::ImageView    view  { VK_NULL_HANDLE };
};

struct BufferResourceRAII {
    vk::raii::DeviceMemory memory { VK_NULL_HANDLE };
    vk::raii::Image        buffer { VK_NULL_HANDLE };
    vk::raii::ImageView    view   { VK_NULL_HANDLE };
    void* mapping = nullptr;
};

class ExampleBaseRAII : public tools::IRenderer {
protected:

    using FrameBufferArray = std::vector<::vk::raii::Framebuffer>;

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

    vk::ImageMemoryBarrier   mCurrSwImBarrier;
 //   RenderPass    mRenderPass;
 //   FrameBufferArray          mFrameBuffers;
 //
 //   VkImageMemoryBarrier      currSwImBarrier; // current swapchain image barrier
 //   uint32_t                  currSwImIndex;
 //   ImageResourceRAII         mDepthStencil;
 //   VkFormat                  mDepthStencilFormat = VK_FORMAT_UNDEFINED;

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
