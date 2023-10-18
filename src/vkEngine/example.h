#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "common.h"
#include "instance.h"
#include "surface.h"
#include "device.h"
#include "swapchain.h"
#include "utils.h"

#include <polyp_window.h>
#include <polyp_logs.h>

#include <array>

namespace polyp {
namespace engine {
namespace example {

using namespace tools;

struct ImageResource {
    DESTROYABLE(VkDeviceMemory) memory { VK_NULL_HANDLE };
    DESTROYABLE(VkImage)        image { VK_NULL_HANDLE };
    DESTROYABLE(VkImageView)    view { VK_NULL_HANDLE };
};

struct BufferResource {
    DESTROYABLE(VkDeviceMemory) memory { VK_NULL_HANDLE };
    DESTROYABLE(VkBuffer)       buffer { VK_NULL_HANDLE };
    DESTROYABLE(VkBufferView)   view { VK_NULL_HANDLE };
    void* mapping = nullptr;
};

class ExampleBase : public tools::IRenderer {
protected:
    using FrameBufferArray = std::vector<DESTROYABLE(VkFramebuffer)>;

    Device::Ptr               mDevice;
    Swapchain::Ptr            mSwapchain;
    VkQueue                   mQueue;
    VkCommandBuffer           mCmdBuffer;
    DESTROYABLE(VkSemaphore)  mReadyToPresent;
    DESTROYABLE(VkFence)      mSubmitFence;
    VkImageMemoryBarrier      currSwImBarrier; // current swapchain image barrier
    uint32_t                  currSwImIndex;
    ImageResource             mDepthStencil;
    VkFormat                  mDepthStencilFormat = VK_FORMAT_UNDEFINED;
    DESTROYABLE(VkRenderPass) mRenderPass;
    FrameBufferArray          mFrameBuffers;

    void preDraw();
    void postDraw();

public:
    virtual ~ExampleBase() override { }

    virtual bool onInit(WindowInstance inst, WindowHandle hwnd) override;

    virtual bool onResize() override;

    virtual void draw() override;

    virtual void onShoutDown() override;

    virtual bool isReady() override { return true; }
};

} // example
} // engine
} // polyp

#endif // EXAMPLE_H
