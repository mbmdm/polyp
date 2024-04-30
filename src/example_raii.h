#ifndef EXAMPLERAII_H
#define EXAMPLERAII_H

#include "vk_context.h"

#include <polyp_window.h>

namespace polyp {
namespace example {

class ExampleBaseRAII : public tools::IRenderer {
protected:

    using FrameBuffers = std::vector<vulkan::Framebuffer>;
    using Images       = std::vector<vk::Image>;
    using Views        = std::vector<vulkan::ImageView>;

    vulkan::Queue          mQueue          = { VK_NULL_HANDLE };
    vulkan::CommandPool    mCmdPool        = { VK_NULL_HANDLE };
    vulkan::CommandBuffer  mCmdBuffer      = { VK_NULL_HANDLE };
    vulkan::Semaphore      mReadyToPresent = { VK_NULL_HANDLE };
    vulkan::Fence          mSubmitFence    = { VK_NULL_HANDLE };
    vulkan::Fence          mAqImageFence   = { VK_NULL_HANDLE };
    vulkan::RenderPass     mRenderPass     = { VK_NULL_HANDLE };

    vk::ImageMemoryBarrier   mCurrSwImBarrier;
    uint32_t                 mCurrSwImIndex = {};

    Images                   mSwapChainImages;
    Views                    mSwapChainVeiews;
    FrameBuffers             mFrameBuffers;

    struct {
        vulkan::Image    image = VK_NULL_HANDLE;
        vulkan::ImageView view = VK_NULL_HANDLE;
    } mDepthStencil;

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
