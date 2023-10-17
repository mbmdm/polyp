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

class ExampleBase : public tools::IRenderer {
private:
    Device::Ptr              mDevice;
    Swapchain::Ptr           mSwapchain;
    VkQueue                  mQueue;
    VkCommandBuffer          mCmdBuffer;
    DESTROYABLE(VkSemaphore) mReadyToPresent;
    DESTROYABLE(VkFence)     mSubmitFence;
    VkImageMemoryBarrier     currSwImBarrier; // current swapchain image barrier
    uint32_t                 currSwImIndex;

protected:
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
