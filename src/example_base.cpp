#include "example_base.h"

#include "vk_common.h"
#include "vk_utils.h"
#include "vk_stringise.h"

using polyp::vulkan::RHIContext;

namespace polyp {
namespace example {

using namespace polyp::vulkan;

void ExampleBase::preDraw() {

    const auto& swapchain = RHIContext::get().swapchain();

    auto [res, imIdx] = swapchain.acquireNextImage(constants::kFenceTimeout, VK_NULL_HANDLE, *mAqImageFence);
    if (res !=  vk::Result::eSuccess ) {
        POLYPFATAL("Failed to get Swapchain images");
    }
    res = vulkan::RHIContext::get().device().waitForFences(*mAqImageFence, VK_TRUE, constants::kFenceTimeout);
    if (res != vk::Result::eSuccess) {
        POLYPFATAL("Failed get nex swapchain image with result %s", vk::to_string(res).c_str());
    }
    res = mAqImageFence.getStatus();
    if (res != vk::Result::eSuccess)
        POLYPFATAL("Unexpected VkFence wait result %s", vk::to_string(res).c_str());

    vulkan::RHIContext::get().device().resetFences(*mAqImageFence);

    mCurrSwImIndex = imIdx;

    vk::CommandBufferBeginInfo beginInfo{ ::vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
    mCmdBuffer.begin(beginInfo);

    mCurrSwImBarrier.image         = mSwapChainImages[imIdx];
    mCurrSwImBarrier.srcAccessMask = vk::AccessFlagBits::eNone;
    mCurrSwImBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
    mCurrSwImBarrier.oldLayout     = vk::ImageLayout::eUndefined;
    mCurrSwImBarrier.newLayout     = vk::ImageLayout::eColorAttachmentOptimal;

    mCmdBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::DependencyFlags(), nullptr, nullptr, mCurrSwImBarrier);
}

void ExampleBase::postDraw() {
    const auto& swapchain = RHIContext::get().swapchain();

    vk::Result res = vk::Result::eSuccess;

    mCurrSwImBarrier.srcAccessMask = mCurrSwImBarrier.dstAccessMask;
    mCurrSwImBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    mCurrSwImBarrier.oldLayout = mCurrSwImBarrier.newLayout;
    mCurrSwImBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;

    mCmdBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::DependencyFlags(), nullptr, nullptr, mCurrSwImBarrier);

    mCmdBuffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*mCmdBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &*mReadyToPresent;
    mQueue.submit(submitInfo, *mSubmitFence);

    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &*mReadyToPresent;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &*swapchain;
    presentInfo.pImageIndices = &mCurrSwImIndex;
    res = mQueue.presentKHR(presentInfo);
    if (res != vk::Result::eSuccess)
        POLYPFATAL("Present failed with result %s", vk::to_string(res).c_str());

    res = vulkan::RHIContext::get().device().waitForFences(*mSubmitFence, VK_TRUE, constants::kFenceTimeout);
    if (res != vk::Result::eSuccess)
        POLYPFATAL("Unexpected VkFence wait result %s", vk::to_string(res).c_str());

    res = mSubmitFence.getStatus();
    if (res != vk::Result::eSuccess)
        POLYPFATAL("Unexpected VkFence wait result %s", vk::to_string(res).c_str());

    vulkan::RHIContext::get().device().resetFences(*mSubmitFence);
}

bool ExampleBase::onInit(const WindowInitializedEventArgs& args)
{
    POLYPDEBUG(__FUNCTION__);

    auto inst = args.windowInstance;
    auto hwnd = args.windowHandle;

    auto& ctx = vulkan::RHIContext::get();

    std::vector<RHIContext::CreateInfo::Queue> queInfos{ {1, vk::QueueFlagBits::eGraphics} };

    RHIContext::CreateInfo ctxInfo {
         {__FILE__, 1},                         // CreateInfo::Application
         RHIContext::CreateInfo::GPU::Powerful, // CreateInfo::GPU
         {hwnd, inst},                          // CreateInfo::Surface
         {queInfos, {}},                        // CreateInfo::Device
         {3},                                   // CreateInfo::SwapChain
    };

    ctx.init(ctxInfo);

    if (!ctx.ready()) {
        POLYPFATAL("Failed to initialize Vulkan infrastructure.");
        return false;
    }

    POLYPINFO("Device %s will be used", ctx.gpu().toStringPLP().c_str());

    const auto& device = ctx.device();

    auto familyIdx = ctx.queueFamily(vk::QueueFlagBits::eGraphics);

    mQueue = ctx.device().getQueue(familyIdx, 0);
    if (*mQueue == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create vulkan graphics queue");
    }
    POLYPINFO("Graphics queue retrieved successfully");

    vk::CommandPoolCreateInfo cmdPoolCreateInfo{};
    cmdPoolCreateInfo.queueFamilyIndex = familyIdx;
    cmdPoolCreateInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    mCmdPool = ctx.device().createCommandPool(cmdPoolCreateInfo);
    if (*mCmdPool == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create command pool");
    }

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool        = *mCmdPool;
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    auto cmds = device.allocateCommandBuffers(allocInfo);
    if (cmds.empty() && *cmds[0] != VK_NULL_HANDLE) {
        POLYPFATAL("Failed to allocate command buffers.");
        return false;
    }

    mCmdBuffer = std::move(cmds[0]);
    POLYPINFO("Primary command buffer created successfully");

    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    mReadyToPresent = device.createSemaphore(semaphoreCreateInfo);
    if (*mReadyToPresent == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create image available semaphore.");
    }

    vk::FenceCreateInfo fenceCreateInfo{/*vk::FenceCreateFlagBits::eSignaled*/};
    mSubmitFence  = device.createFence(fenceCreateInfo);
    mAqImageFence = device.createFence(fenceCreateInfo);
    if (*mSubmitFence == VK_NULL_HANDLE || *mAqImageFence == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create fence.");
    }

    POLYPINFO("Semaphores and fence created successfully");

    mCurrSwImBarrier = vk::ImageMemoryBarrier{};

    mCurrSwImBarrier.srcAccessMask       = vk::AccessFlagBits::eNone;
    mCurrSwImBarrier.dstAccessMask       = vk::AccessFlagBits::eNone;
    mCurrSwImBarrier.oldLayout           = vk::ImageLayout::eUndefined;
    mCurrSwImBarrier.newLayout           = vk::ImageLayout::eUndefined;
    mCurrSwImBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mCurrSwImBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mCurrSwImBarrier.image               = VK_NULL_HANDLE;

    mCurrSwImBarrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    mCurrSwImBarrier.subresourceRange.baseMipLevel   = 0;
    mCurrSwImBarrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
    mCurrSwImBarrier.subresourceRange.baseArrayLayer = 0;
    mCurrSwImBarrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;

    onResize();

    return true;
}

bool ExampleBase::onResize() {
    POLYPDEBUG(__FUNCTION__);

    auto& ctx = vulkan::RHIContext::get();

    const auto& device    = ctx.device();
    const auto& gpu       = ctx.gpu();
    const auto& surface   = ctx.surface();
    const auto& swapchain = ctx.swapchain();

    device.waitIdle();
    ctx.onResize();
    
    auto [image, view] = utils::createDepthStencil();
    mDepthStencil.image = std::move(image);
    mDepthStencil.view  = std::move(view);

    mRenderPass = utils::createRenderPass();

    mSwapChainImages = swapchain.getImages();

    mSwapChainVeiews.clear();
    mFrameBuffers.clear();

    for (auto i = 0; i < mSwapChainImages.size(); ++i)
    {
        vk::ImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
        viewCreateInfo.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        viewCreateInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        viewCreateInfo.subresourceRange.baseMipLevel   = 0;
        viewCreateInfo.subresourceRange.levelCount     = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount     = 1;
        viewCreateInfo.viewType                        = vk::ImageViewType::e2D;
        viewCreateInfo.flags                           = {};
        viewCreateInfo.image                           = mSwapChainImages[i];

        auto view = vulkan::RHIContext::get().device().createImageView(viewCreateInfo);
        mSwapChainVeiews.push_back(std::move(view));

        std::array<vk::ImageView, 2> attachments;
        attachments[0] = *mSwapChainVeiews[i];
        attachments[1] = *mDepthStencil.view;

        auto capabilities = vulkan::RHIContext::get().gpu().getSurfaceCapabilitiesKHR(*ctx.surface());

        vk::FramebufferCreateInfo fbCreateInfo{};
        fbCreateInfo.renderPass      = *mRenderPass;
        fbCreateInfo.attachmentCount = attachments.size();
        fbCreateInfo.pAttachments    = attachments.data();
        fbCreateInfo.width           = capabilities.currentExtent.width;
        fbCreateInfo.height          = capabilities.currentExtent.height;
        fbCreateInfo.layers          = 1;

        auto fb = device.createFramebuffer(fbCreateInfo);
        mFrameBuffers.push_back(std::move(fb));
    }

    return true;
}

void ExampleBase::draw() {
    preDraw();
    POLYPDEBUG(__FUNCTION__);
    postDraw();
}

void ExampleBase::onShoutDown() {
    POLYPDEBUG(__FUNCTION__);
    vulkan::RHIContext::get().device().waitIdle();
}

} // example
} // polyp
