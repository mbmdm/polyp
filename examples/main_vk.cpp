#include <camera.h>
#include <error_codes.h>
#include <instance.h>
#include <surface.h>
#include <device.h>
#include <swapchain.h>
#include <utils.h>
#include <polyp_window.h>
#include <polyp_logs.h>

#include <thread>
#include <chrono>

using namespace polyp::engine;
using namespace polyp::tools;

constexpr auto gFenceTimeout = 2'000'000'000;
uint64_t       gFrameCounter = 0;

namespace {

void beginCmd(Device::ConstPtr device, VkCommandBuffer cmd) {
    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CHECKRET(device->vk().BeginCommandBuffer(cmd, &beginInfo));
}

void endCmd(Device::ConstPtr device, VkCommandBuffer cmd) {
    CHECKRET(device->vk().EndCommandBuffer(cmd));
}

void queueSubmit(Device::ConstPtr device, VkCommandBuffer cmd, VkQueue queue, 
                 VkSemaphore toSignaleSemaphore, VkFence fence) {
    VkSubmitInfo submitIinfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,   // VkStructureType                sType
        nullptr,                         // const void                   * pNext
        0,                               // uint32_t                       waitSemaphoreCount
        nullptr,                         // const VkSemaphore            * pWaitSemaphores
        nullptr,                         // const VkPipelineStageFlags   * pWaitDstStageMask
        1,                               // uint32_t                       commandBufferCount
        &cmd,                            // const VkCommandBuffer        * pCommandBuffers
        1,                               // uint32_t                       signalSemaphoreCount
        &toSignaleSemaphore              // const VkSemaphore            * pSignalSemaphores
    };
    CHECKRET(device->vk().QueueSubmit(queue, 1, &submitIinfo, fence));
}

void present(Device::ConstPtr device, VkQueue queue, VkSwapchainKHR swapchain, 
             VkSemaphore waitSemaphore, uint32_t imageIndex ) {
    VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                   // VkStructureType          sType
        nullptr,                                              // const void*              pNext
        1,                                                    // uint32_t                 waitSemaphoreCount
        &waitSemaphore,                                    // const VkSemaphore      * pWaitSemaphores
        1,                                                    // uint32_t                 swapchainCount
        &swapchain,                                           // const VkSwapchainKHR   * pSwapchains
        &imageIndex,                                          // const uint32_t         * pImageIndices
        nullptr                                               // VkResult*                pResults
    };
    CHECKRET(device->vk().QueuePresentKHR(queue, &presentInfo));
}

struct ImageTransition {
    VkImage             Image;
    VkAccessFlags       CurrentAccess;
    VkAccessFlags       NewAccess;
    VkImageLayout       CurrentLayout;
    VkImageLayout       NewLayout;
    uint32_t            CurrentQueueFamily;
    uint32_t            NewQueueFamily;
    VkImageAspectFlags  Aspect;
};

void setImMemoryBarrier(Device::ConstPtr device, VkCommandBuffer cmd, VkPipelineStageFlags generatingStages, 
                        VkPipelineStageFlags consumingStages, ImageTransition imTransitions) {
    VkImageMemoryBarrier imBarrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,   // VkStructureType            sType
        nullptr,                                  // const void               * pNext
        imTransitions.CurrentAccess,              // VkAccessFlags              srcAccessMask
        imTransitions.NewAccess,                  // VkAccessFlags              dstAccessMask
        imTransitions.CurrentLayout,              // VkImageLayout              oldLayout
        imTransitions.NewLayout,                  // VkImageLayout              newLayout
        imTransitions.CurrentQueueFamily,         // uint32_t                   srcQueueFamilyIndex
        imTransitions.NewQueueFamily,             // uint32_t                   dstQueueFamilyIndex
        imTransitions.Image,                      // VkImage                    image
        {                                         // VkImageSubresourceRange    subresourceRange
          imTransitions.Aspect,                   // VkImageAspectFlags         aspectMask
          0,                                      // uint32_t                   baseMipLevel
          VK_REMAINING_MIP_LEVELS,                // uint32_t                   levelCount
          0,                                      // uint32_t                   baseArrayLayer
          VK_REMAINING_ARRAY_LAYERS               // uint32_t                   layerCount
        }
    };
    device->vk().CmdPipelineBarrier(cmd, generatingStages, consumingStages, 0, 0, nullptr, 0, nullptr, 1, &imBarrier);
}

} // anonimus namespace

class Sample : public polyp::tools::IRenderer {
private:
    Device::Ptr              mDevice;
    Swapchain::Ptr           mSwapchain;
    VkQueue                  mQueue;
    VkCommandBuffer          mCmdBuffer;
    DESTROYABLE(VkFence)     mSubmitFence;
    DESTROYABLE(VkSemaphore) mReadyToPresent;

public:
    virtual ~Sample() override { }

    virtual bool onInit(WindowInstance inst, WindowHandle hwnd) override {
        POLYPINFO(__FUNCTION__);

        InstanceCreateInfo instanceInfo;
        auto instance = Instance::create();
        if (!instance) {
            POLYPFATAL("Failed to create vulkan instance");
            return false;
        }

        polyp::engine::PhysicalGpu physGpu;
        for (size_t i = 0; i < instance->gpuCount(); i++) {
            auto gpu = instance->gpu(i);
            if (gpu.memory() > physGpu.memory() && gpu.isDiscrete()) {
                physGpu = gpu;
            }
        }
        POLYPINFO("Selected device %s with local memory %d mb\n", physGpu.name().c_str(), physGpu.memory() / 1024 / 1024);

        SurfaceCreateInfo info{ inst, hwnd };
        Surface::Ptr surface = Surface::create(instance, info);
        if (!surface) {
            POLYPFATAL("Failed to create vulkan surface (WSI)");
            return false;
        }

        QueueCreateInfo queInfo;
        queInfo.mFamilyIndex = UINT32_MAX;
        queInfo.mQueueType = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
        queInfo.mPriorities = { 1., 0.5 };

        auto availableGraphicsQueue = physGpu.checkSupport(queInfo.mQueueType, queInfo.mPriorities.size());
        auto availableWSIQueue = surface->checkSupport(physGpu);

        for (uint32_t i = 0; i < physGpu.queueFamilyCount(); i++) {
            if (availableGraphicsQueue[i] && availableWSIQueue[i]) {
                queInfo.mFamilyIndex = i;
                break;
            }
        }
        if (queInfo.mFamilyIndex == ~0) {
            POLYPFATAL("Failed to find the reqired queue family");
            return false;
        }

        DeviceCreateInfo deviceInfo;
        queInfo.mFamilyIndex = UINT32_MAX;
        deviceInfo.mQueueInfo = { queInfo };
        mDevice = Device::create(instance, physGpu, deviceInfo);
        if (!mDevice) {
            POLYPFATAL("Failed to create vulkan rendering device");
            return false;
        }

        mQueue = mDevice->queue(VK_QUEUE_GRAPHICS_BIT, 0.89);
        mCmdBuffer = mDevice->cmdBuffer(mQueue, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        SwapChainCreateInfo swInfo;
        swInfo.presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
        mSwapchain = Swapchain::create(mDevice, surface, swInfo);
        if (!mSwapchain) {
            POLYPFATAL("Failed to create vulkan swap chain.");
            return false;
        }

        VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        semaphoreCreateInfo.flags = 0;
        CHECKRET(mDevice->vk().CreateSemaphore(**mDevice, &semaphoreCreateInfo, nullptr, mReadyToPresent.pNative()));
        mReadyToPresent.initDestroyer(mDevice);

        *mSubmitFence = utils::createFence(mDevice);
        mSubmitFence.initDestroyer(mDevice);

        return true;
    }

    virtual bool onResize() override {
        POLYPINFO(__FUNCTION__);
        return mSwapchain->update();
    }

    virtual void onShoutDown() override {
        mDevice->vk().DeviceWaitIdle(mDevice->native());
        POLYPTODO(
            "Need association cmdBuffer and cmdPool to have an ability to release cmdBuffer with vkFreeCommandBuffers()"
            "Seems I should move such login in device class and someway point out that native handles shouldn't be "
            "desctoyed manually. I can also have logic of recreatiion objects with different flags"
        );
        POLYPTODO(
            "Command buffers allocated from a given pool are implicitly freed when we destroy a"
            "command pool. So when we want to destroy a pool, we don't need to separately free all"
            "command buffers allocated from it. ((c) VulkanCookBook)."
        );
        POLYPTODO("Seems I shouldn't descroy them, but it can be useful in recreation approach");
        POLYPINFO(__FUNCTION__);
    }

    virtual bool isReady() override { return true; }

    virtual void draw() override {
        POLYPINFO("Frame %lu", gFrameCounter++);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto [im, imIdx] = mSwapchain->nextImage();
        if (im == VK_NULL_HANDLE) {
            POLYPFATAL("Failed to get Swapchain images");
        }

        beginCmd(mDevice, mCmdBuffer);
        ImageTransition imTransitionBeforeDraw = {
             im,                                       // VkImage              Image
             0,                                        // VkAccessFlags        CurrentAccess
             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,     // VkAccessFlags        NewAccess
             VK_IMAGE_LAYOUT_UNDEFINED,                // VkImageLayout        CurrentLayout
             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout        NewLayout
             VK_QUEUE_FAMILY_IGNORED,                  // uint32_t             CurrentQueueFamily
             VK_QUEUE_FAMILY_IGNORED,                  // uint32_t             NewQueueFamily
             VK_IMAGE_ASPECT_COLOR_BIT                 // VkImageAspectFlags   Aspect
        };
        setImMemoryBarrier(mDevice, mCmdBuffer, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, imTransitionBeforeDraw);

        ImageTransition imTransitionBeforePresent = {
            im,                                       // VkImage              Image
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,     // VkAccessFlags        CurrentAccess
            VK_ACCESS_MEMORY_READ_BIT,                // VkAccessFlags        NewAccess
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // VkImageLayout        CurrentLayout
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,          // VkImageLayout        NewLayout
            VK_QUEUE_FAMILY_IGNORED,                  // uint32_t             CurrentQueueFamily
            VK_QUEUE_FAMILY_IGNORED,                  // uint32_t             NewQueueFamily
            VK_IMAGE_ASPECT_COLOR_BIT                 // VkImageAspectFlags   Aspect
        };
        setImMemoryBarrier(mDevice, mCmdBuffer, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, imTransitionBeforePresent);
        endCmd(mDevice, mCmdBuffer);
        
        queueSubmit(mDevice, mCmdBuffer, mQueue, *mReadyToPresent, *mSubmitFence);
        present(mDevice, mQueue, mSwapchain->native(), *mReadyToPresent, imIdx);

        // Waiting for pending cmdBuffer
        CHECKRET(mDevice->vk().WaitForFences(**mDevice, 1, &*mSubmitFence, VK_TRUE, gFenceTimeout));
        CHECKRET(mDevice->vk().GetFenceStatus(**mDevice, *mSubmitFence));
        CHECKRET(mDevice->vk().ResetFences(**mDevice, 1, &*mSubmitFence));
    }
};

int main() {
    IRenderer::Ptr sample = std::make_shared<Sample>();
    PolypWindow win{ polyp::constants::kWindowTitle, 0, 0, 1024, 600, sample };
    win.run();
}
