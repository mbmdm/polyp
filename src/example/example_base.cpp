#include "example_base.h"
#include "vk_utils.h"

namespace polyp {
namespace vulkan {
namespace example {


void ExampleBase::onRender()
{
    if (mPauseDrawing)
        return;

    acquireNextSwapChainImage();
    waitForFence();
    draw();
    submit();
    present();
}

bool ExampleBase::onInit(const WindowInitializedEventArgs& args)
{
    mContextInfo = getRHICreateInfo();

    if (mContextInfo.win.instance == NULL)
        mContextInfo.win.instance = args.windowInstance;

    if (mContextInfo.win.handle == NULL)
        mContextInfo.win.handle   = args.windowHandle;

    auto& ctx = RHIContext::get();

    ctx.init(mContextInfo);

    if (!ctx.ready()) {
        POLYPFATAL("Failed to initialize Vulkan infrastructure.");
        return false;
    }

    POLYPINFO("Device [%s] will be used", ctx.gpu().toStringPLP().c_str());

    const auto& device = ctx.device();

    auto familyIdx = ctx.queueFamily(mContextInfo.device.queues[0].flags);

    mQueue = ctx.device().getQueue(familyIdx, 0);
    if (*mQueue == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create vulkan graphics queue");
    }
    POLYPDEBUG("Graphics queue retrieved successfully");

    vk::CommandPoolCreateInfo cmdPoolCreateInfo{};
    cmdPoolCreateInfo.queueFamilyIndex = familyIdx;
    cmdPoolCreateInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    mCmdPool = ctx.device().createCommandPool(cmdPoolCreateInfo);
    if (*mCmdPool == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create command pool");
    }

    mAqImageFence = utils::createFence();
    if (*mAqImageFence == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create fence.");
    }

    WindowResizeEventArgs resizeArgs {
        .mode = WindowResizeMode::Restored
    };

    onResize(resizeArgs);

    createDrawCmds();
    
    if (mDrawCmds.size() != mSwapChainImages.size()) {
        POLYPFATAL("Failed to create command buffers and fences.");
    }

    if (!postInit())
        POLYPFATAL("Post initialization failed.");

    return true;
}

bool ExampleBase::onResize(const WindowResizeEventArgs& args)
{
    if (args.mode == WindowResizeMode::Minimized) {
        mPauseDrawing = true;
        return true;
    }
    
    mPauseDrawing = false;

    const auto& device    = RHIContext::get().device();
    const auto& swapchain = RHIContext::get().swapchain();

    device.waitIdle();
    RHIContext::get().onResize();

    mSwapChainImages = swapchain.getImages();

    mSwapChainVeiews.clear();
    mSemaphores.clear();

    for (auto i = 0; i < mSwapChainImages.size(); ++i)
    {
        vk::ImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.format     = vk::Format::eB8G8R8A8Unorm;
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

        auto view = device.createImageView(viewCreateInfo);
        mSwapChainVeiews.push_back(std::move(view));

        vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        auto semaphore = device.createSemaphore(semaphoreCreateInfo);
        if (*semaphore == VK_NULL_HANDLE) {
            POLYPFATAL("Failed to create semaphore.");
        }
        
        mSemaphores.push_back(std::move(semaphore));
    }

    return postResize();
}

void ExampleBase::OnMouseClick(const MouseClickEventArgs& args)
{
    if (args.button == MouseButton::Left && args.action == MouseActioin::Click)
    {
        mLastXMousePos = 0.f;
        mLastYMousePos = 0.f;
        mMouseMoving = true;
    }

    if (args.button == MouseButton::Left && args.action == MouseActioin::Release)
        mMouseMoving = false;
}

void ExampleBase::onMovement(const MovementEventArgs& args)
{
    auto deltaTime = (1 / mFPSCounter.fps());

    if (args.HasMotion())
    {
        if (args.move.ahead)
            mCamera.processKeyboard(Camera::Direction::Forward, deltaTime);
        else if (args.move.back)
            mCamera.processKeyboard(Camera::Direction::Backward, deltaTime);

        if (args.move.left)
            mCamera.processKeyboard(Camera::Direction::Left, deltaTime);
        else if (args.move.righ)
            mCamera.processKeyboard(Camera::Direction::Right, deltaTime);

        if (args.move.up)
            mCamera.processKeyboard(Camera::Direction::Up, deltaTime);
        else if (args.move.down)
            mCamera.processKeyboard(Camera::Direction::Down, deltaTime);
    }

    if (args.HasMouse() && mMouseMoving)
    {
        if (mLastXMousePos == 0 && mLastYMousePos == 0)
        {
            mLastXMousePos = args.mouse.x;
            mLastYMousePos = args.mouse.y;
            return;
        }

        auto xoffset = args.mouse.x - mLastXMousePos;
        auto yoffset = args.mouse.y - mLastYMousePos;

        mLastXMousePos = args.mouse.x;
        mLastYMousePos = args.mouse.y;

        mCamera.procesMouse(xoffset, yoffset);
    }
}

void ExampleBase::onShoutDown()
{
    RHIContext::get().device().waitIdle();
}

ExampleBase::MVP ExampleBase::getMVP()
{
    MVP output{};

    output.modelMatrix = glm::mat4(1.0f);

    output.viewMatrix = mCamera.view();

    auto& ctx         = vulkan::RHIContext::get();
    auto capabilities = ctx.gpu().getSurfaceCapabilitiesKHR(*ctx.surface());

    const auto width  = capabilities.currentExtent.width;
    const auto height = capabilities.currentExtent.height;

    output.projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    return output;
}

void ExampleBase::acquireNextSwapChainImage()
{
    const auto& swapchain = RHIContext::get().swapchain();

    auto [res, imIdx] = swapchain.acquireNextImage(constants::kFenceTimeout, VK_NULL_HANDLE, *mAqImageFence);
    if (res !=  vk::Result::eSuccess ) {
        POLYPFATAL("Failed acquire the next swapchain image with result %s", vk::to_string(res).c_str());
    }

    res = RHIContext::get().device().waitForFences(*mAqImageFence, VK_TRUE, constants::kFenceTimeout);
    if (res != vk::Result::eSuccess) {
        POLYPFATAL("Failed wait for the next swapchain image with result %s", vk::to_string(res).c_str());
    }

    res = mAqImageFence.getStatus();
    if (res != vk::Result::eSuccess)
        POLYPFATAL("Unexpected VkFence wait result %s", vk::to_string(res).c_str());

    RHIContext::get().device().resetFences(*mAqImageFence);

    mCurrSwImIndex = imIdx;
}

void ExampleBase::createDrawCmds()
{
    const auto size = mSwapChainImages.size();

    mDrawCmds.clear();
    mDrawFences.clear();

    for (size_t i = 0; i < size; ++i)
    {
        auto cmd = utils::createCommandBuffer(mCmdPool, vk::CommandBufferLevel::ePrimary);
        if (*cmd == VK_NULL_HANDLE)
            continue;

        auto fence = utils::createFence(true);
        if (*fence == VK_NULL_HANDLE)
            continue;

        mDrawCmds.push_back(std::move(cmd));
        mDrawFences.push_back(std::move(fence));
    }
}

void ExampleBase::submit()
{
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &*mDrawCmds[mCurrSwImIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = &*mSemaphores[mCurrSwImIndex];
    mQueue.submit(submitInfo, *mDrawFences[mCurrSwImIndex]);
}

void ExampleBase::present()
{
    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &*mSemaphores[mCurrSwImIndex];
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &*RHIContext::get().swapchain();
    presentInfo.pImageIndices      = &mCurrSwImIndex;

    auto res = mQueue.presentKHR(presentInfo);

    mFPSCounter.onPresent();

    if (res != vk::Result::eSuccess)
        POLYPFATAL("Present failed with result %s", vk::to_string(res).c_str());
}

void ExampleBase::waitForFence()
{
    const auto& ctx = RHIContext::get();

    auto res = ctx.device().waitForFences(*mDrawFences[mCurrSwImIndex], VK_TRUE, constants::kFenceTimeout);
    if (res != vk::Result::eSuccess) {
        POLYPFATAL("Unexpected VkFence wait result %s", vk::to_string(res).c_str());
    }

    res = mDrawFences[mCurrSwImIndex].getStatus();
    if (res != vk::Result::eSuccess)
        POLYPFATAL("Unexpected VkFence wait result %s", vk::to_string(res).c_str());

    vulkan::RHIContext::get().device().resetFences(*mDrawFences[mCurrSwImIndex]);
}

} // example
} // vulkan
} // polyp
