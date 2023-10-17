#include "common.h"
#include "destroyer.h"
#include "example.h"

namespace {
constexpr auto gFenceTimeout = 2'000'000'000;
} // anonimus namespace

namespace polyp {
namespace engine {
namespace example {

void ExampleBase::preDraw() {
    auto [im, imIdx] = mSwapchain->nextImage();
    if (im == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to get Swapchain images");
    }

    currSwImIndex = imIdx;

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CHECKRET(mDevice->vk().BeginCommandBuffer(mCmdBuffer, &beginInfo));

    currSwImBarrier.image = im;
    currSwImBarrier.srcAccessMask = 0;
    currSwImBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    currSwImBarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    currSwImBarrier.newLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    mDevice->vk().CmdPipelineBarrier(mCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &currSwImBarrier);
}

void ExampleBase::postDraw() {
    currSwImBarrier.srcAccessMask = currSwImBarrier.dstAccessMask;
    currSwImBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    currSwImBarrier.oldLayout     = currSwImBarrier.newLayout;
    currSwImBarrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    mDevice->vk().CmdPipelineBarrier(mCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &currSwImBarrier);

    CHECKRET(mDevice->vk().EndCommandBuffer(mCmdBuffer));;

    VkSubmitInfo submitIinfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO, // VkStructureType                sType
        nullptr,                       // const void                   * pNext
        0,                             // uint32_t                       waitSemaphoreCount
        nullptr,                       // const VkSemaphore            * pWaitSemaphores
        nullptr,                       // const VkPipelineStageFlags   * pWaitDstStageMask
        1,                             // uint32_t                       commandBufferCount
        &mCmdBuffer,                   // const VkCommandBuffer        * pCommandBuffers
        1,                             // uint32_t                       signalSemaphoreCount
        mReadyToPresent.pNative()      // const VkSemaphore            * pSignalSemaphores
    };
    CHECKRET(mDevice->vk().QueueSubmit(mQueue, 1, &submitIinfo, *mSubmitFence));

    VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // VkStructureType          sType
        nullptr,                            // const void*              pNext
        1,                                  // uint32_t                 waitSemaphoreCount
        mReadyToPresent.pNative(),          // const VkSemaphore      * pWaitSemaphores
        1,                                  // uint32_t                 swapchainCount
        mSwapchain->pNative(),              // const VkSwapchainKHR   * pSwapchains
        &currSwImIndex,                     // const uint32_t         * pImageIndices
        nullptr                             // VkResult*                pResults
    };
    CHECKRET(mDevice->vk().QueuePresentKHR(mQueue, &presentInfo));

    CHECKRET(mDevice->vk().WaitForFences(mDevice->native(), 1, mSubmitFence.pNative(), VK_TRUE, gFenceTimeout));
    CHECKRET(mDevice->vk().GetFenceStatus(mDevice->native(), mSubmitFence.native()));
    CHECKRET(mDevice->vk().ResetFences(mDevice->native(), 1, mSubmitFence.pNative()));
}

bool ExampleBase::onInit(WindowInstance inst, WindowHandle hwnd) {
    POLYPDEBUG(__FUNCTION__);

    auto instance = Instance::create();
    if (!instance) {
        POLYPFATAL("Failed to create vulkan instance");
        return false;
    }

    if (instance->gpuCount() == 0) {
        POLYPFATAL("Required GPU not found");
        return false;
    }

    auto physGpu = instance->gpu(0);
    for (size_t i = 1; i < instance->gpuCount(); i++) {
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

    currSwImBarrier = VkImageMemoryBarrier{
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // VkStructureType            sType
    nullptr,                                // const void               * pNext
    0,                                      // VkAccessFlags              srcAccessMask
    0,                                      // VkAccessFlags              dstAccessMask
    VK_IMAGE_LAYOUT_UNDEFINED,              // VkImageLayout              oldLayout
    VK_IMAGE_LAYOUT_UNDEFINED,              // VkImageLayout              newLayout
    VK_QUEUE_FAMILY_IGNORED,                // uint32_t                   srcQueueFamilyIndex
    VK_QUEUE_FAMILY_IGNORED,                // uint32_t                   dstQueueFamilyIndex
    VK_NULL_HANDLE,                         // VkImage                    image
    {                                       // VkImageSubresourceRange    subresourceRange
      VK_IMAGE_ASPECT_COLOR_BIT,            // VkImageAspectFlags         aspectMask
      0,                                    // uint32_t                   baseMipLevel
      VK_REMAINING_MIP_LEVELS,              // uint32_t                   levelCount
      0,                                    // uint32_t                   baseArrayLayer
      VK_REMAINING_ARRAY_LAYERS             // uint32_t                   layerCount
    } };



    return true;
}

bool ExampleBase::onResize() {
    POLYPDEBUG(__FUNCTION__);
    return mSwapchain->update();
}

void ExampleBase::draw() {
    preDraw();
    POLYPDEBUG(__FUNCTION__);
    postDraw();
}

void ExampleBase::onShoutDown() {
    POLYPDEBUG(__FUNCTION__);
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
}

} // example
} // engine
} // polyp
