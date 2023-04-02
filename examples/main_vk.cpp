#include <camera.h>
#include <error_codes.h>
#include <instance.h>
#include <surface.h>
#include <device.h>
#include <swapchain.h>
#include <utils.h>
#include <window_surface.h>
#include <polyp_logs.h>

#include <thread>
#include <chrono>

using namespace polyp::engine;
using namespace polyp::tools;

class Sample : public polyp::tools::IRenderer {
private:
    Instance::Ptr                    mInstance;
    Device::Ptr                      mDevice;
    Swapchain::Ptr                   mSwapchain;
    VkQueue                          mQueue;
    VkCommandBuffer                  mCmdBuffer;
    DECLARE_VKDESTROYER(VkSemaphore) mReadyToPresent;

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

    void beginCmd() {
        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        CHECKRET(mDevice->vk().BeginCommandBuffer(mCmdBuffer, &beginInfo));
    }

    void endCmd() {
        CHECKRET(mDevice->vk().EndCommandBuffer(mCmdBuffer));
    }

    void setImMemoryBarrier(VkPipelineStageFlags generatingStages, 
                            VkPipelineStageFlags consumingStages, 
                            std::vector<ImageTransition> imTransitions) {
        std::vector<VkImageMemoryBarrier> barriers;

        for (auto& transition : imTransitions) {
            barriers.push_back({
              VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,   // VkStructureType            sType
              nullptr,                                  // const void               * pNext
              transition.CurrentAccess,                 // VkAccessFlags              srcAccessMask
              transition.NewAccess,                     // VkAccessFlags              dstAccessMask
              transition.CurrentLayout,                 // VkImageLayout              oldLayout
              transition.NewLayout,                     // VkImageLayout              newLayout
              transition.CurrentQueueFamily,            // uint32_t                   srcQueueFamilyIndex
              transition.NewQueueFamily,                // uint32_t                   dstQueueFamilyIndex
              transition.Image,                         // VkImage                    image
              {                                         // VkImageSubresourceRange    subresourceRange
                transition.Aspect,                      // VkImageAspectFlags         aspectMask
                0,                                      // uint32_t                   baseMipLevel
                VK_REMAINING_MIP_LEVELS,                // uint32_t                   levelCount
                0,                                      // uint32_t                   baseArrayLayer
                VK_REMAINING_ARRAY_LAYERS               // uint32_t                   layerCount
              }
                });
        }
        if (!barriers.empty()) {
            mDevice->vk().CmdPipelineBarrier(mCmdBuffer, generatingStages, consumingStages,
                0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data());
        }
    }

    void queueSubmit() {
        VkSubmitInfo submitIinfo = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,                        // VkStructureType                sType
            nullptr,                                              // const void                   * pNext
            0,                                                    // uint32_t                       waitSemaphoreCount
            nullptr,                                              // const VkSemaphore            * pWaitSemaphores
            nullptr,                                              // const VkPipelineStageFlags   * pWaitDstStageMask
            1,                                                    // uint32_t                       commandBufferCount
            &mCmdBuffer,                                          // const VkCommandBuffer        * pCommandBuffers
            1,                                                    // uint32_t                       signalSemaphoreCount
            &*mReadyToPresent                                     // const VkSemaphore            * pSignalSemaphores
        };
        CHECKRET(mDevice->vk().QueueSubmit(mQueue, 1, &submitIinfo, VK_NULL_HANDLE));
    }

    void present(uint32_t imIdx) {
        VkPresentInfoKHR presentInfo = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,                   // VkStructureType          sType
            nullptr,                                              // const void*              pNext
            1,                                                    // uint32_t                 waitSemaphoreCount
            &*mReadyToPresent,                                    // const VkSemaphore      * pWaitSemaphores
            1,                                                    // uint32_t                 swapchainCount
            &**mSwapchain,                                        // const VkSwapchainKHR   * pSwapchains
            &imIdx,                                               // const uint32_t         * pImageIndices
            nullptr                                               // VkResult*                pResults
        };
        CHECKRET(mDevice->vk().QueuePresentKHR(mQueue, &presentInfo));
    }

public:
    virtual ~Sample() override {
        POLYPINFO(__FUNCTION__);
    }

    virtual bool onInit(WindowsInstance inst, WindowsHandle hwnd) override {
        POLYPINFO(__FUNCTION__);

        InstanceCreateInfo instanceInfo;
        mInstance = Instance::create();
        if (!mInstance) {
            POLYPFATAL("Failed to create vulkan instance");
            return false;
        }

        polyp::engine::PhysicalGpu physGpu;
        for (size_t i = 0; i < mInstance->gpuCount(); i++) {
            auto gpu = mInstance->gpu(i);
            if (gpu.memory() > physGpu.memory() && gpu.isDiscrete()) {
                physGpu = gpu;
            }
        }
        POLYPINFO("Selected device %s with local memory %d mb\n", physGpu.name().c_str(), physGpu.memory() / 1024 / 1024);

        SurfaceCreateInfo info{ inst, hwnd };
        Surface::Ptr surface = Surface::create(mInstance, info);
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
        mDevice = Device::create(mInstance, physGpu, deviceInfo);
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

        mReadyToPresent = { {**mDevice, VK_NULL_HANDLE}, nullptr };
        VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        semaphoreCreateInfo.flags = 0;
        CHECKRET(mDevice->vk().CreateSemaphore(**mDevice, &semaphoreCreateInfo, nullptr, &*mReadyToPresent));
        initVkDestroyer(mDevice->vk().DestroySemaphore, mReadyToPresent, nullptr);

        return true;
    }

    virtual bool onResize() override {
        POLYPINFO(__FUNCTION__);
        mSwapchain->update();
        return true;
    }

    virtual void onMouseClick(uint32_t button, bool state) override {
        POLYPINFO(__FUNCTION__);
    }

    virtual void onMouseMove(int x, int y) override {
        //POLYPINFO(__FUNCTION__);
    }

    virtual void onMouseWheel(float value) override {
        POLYPINFO(__FUNCTION__);
    }

    virtual void onShoutDown() override {
        POLYPINFO(__FUNCTION__);
    }

    virtual bool isReady() override {
        //POLYPINFO(__FUNCTION__);
        return true;
    }

    virtual void draw() override {
        static int drawCounter = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        CHECKRET(mDevice->vk().DeviceWaitIdle(**mDevice));

        auto [im, imIdx] = mSwapchain->nextImage();
        if (im == VK_NULL_HANDLE) {
            POLYPFATAL("Failed to get Swapchain images");
        }

        beginCmd();
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
        setImMemoryBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { imTransitionBeforeDraw });

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
        setImMemoryBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, { imTransitionBeforePresent });
        endCmd();
        
        queueSubmit();
        present(imIdx);
        
        POLYPINFO("Frame %lu", drawCounter);
        drawCounter++;
    }
    
    virtual void updateTimer() override {
        //POLYPINFO(__FUNCTION__);
    }

    virtual void mouseReset() override {
        //POLYPINFO(__FUNCTION__);
    }
};

int main() {

    IRenderer::Ptr sample = std::make_shared<Sample>();
    WindowSurface win{ polyp::constants::kWindowTitle, 0, 0, 1024, 600, sample };
    win.run();
}
