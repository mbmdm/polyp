#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "common.h"
#include "instance.h"
#include "surface.h"
#include "device.h"
#include "swapchain.h"
#include "utils.h"
#include "destroyable_handle.h"

#include <polyp_window.h>
#include <polyp_logs.h>

#include <array>

namespace polyp {
namespace vk {
namespace example {

using namespace tools;

struct ImageResource {
    DestroyableHandle<VkDeviceMemory> memory { VK_NULL_HANDLE };
    DestroyableHandle<VkImage>        image { VK_NULL_HANDLE };
    DestroyableHandle<VkImageView>    view { VK_NULL_HANDLE };
};

struct BufferResource {
    DestroyableHandle<VkDeviceMemory> memory { VK_NULL_HANDLE };
    DestroyableHandle<VkBuffer>       buffer { VK_NULL_HANDLE };
    DestroyableHandle<VkBufferView>   view { VK_NULL_HANDLE };
    void* mapping = nullptr;
};

class ExampleBase : public tools::IRenderer {
protected:
    using FrameBufferArray = std::vector<DestroyableHandle<VkFramebuffer>>;

    Device::Ptr               mDevice;
    Swapchain::Ptr            mSwapchain;
    VkQueue                   mQueue;
    VkCommandBuffer           mCmdBuffer;
    DestroyableHandle<VkSemaphore>  mReadyToPresent;
    DestroyableHandle<VkFence>      mSubmitFence;
    VkImageMemoryBarrier      currSwImBarrier; // current swapchain image barrier
    uint32_t                  currSwImIndex;
    ImageResource             mDepthStencil;
    VkFormat                  mDepthStencilFormat = VK_FORMAT_UNDEFINED;
    DestroyableHandle<VkRenderPass> mRenderPass;
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
