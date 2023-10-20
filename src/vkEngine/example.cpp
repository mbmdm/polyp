#include "common.h"
#include "example.h"

#include <constants.h>

namespace {
using namespace polyp::engine;
using namespace polyp::constants;

auto depthFormat(Device::ConstPtr device) {
    std::vector<VkFormat> dsDesiredFormats = {
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM_S8_UINT,
    VK_FORMAT_D16_UNORM
    };

    VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    for (const auto& format : dsDesiredFormats) {
        VkFormatProperties formatProps;
        auto instance = device->instance();
        instance->vk().GetPhysicalDeviceFormatProperties(*device->gpu(), format, &formatProps);
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            depthFormat = format;
            break;
        }
    }
    POLYPASSERT(depthFormat != VK_FORMAT_UNDEFINED);

    return depthFormat;
}

auto createDepthStencil(Device::ConstPtr device, Surface::ConstPtr surface) {
    VkImage image         = VK_NULL_HANDLE;
    VkImageView view      = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    const auto width  = surface->width(device->gpu());
    const auto height = surface->height(device->gpu());

    VkImageCreateInfo imCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imCreateInfo.format = depthFormat(device);
    imCreateInfo.extent = { width, height, 1 };
    imCreateInfo.mipLevels = 1;
    imCreateInfo.arrayLayers = 1;
    imCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    CHECKRET(device->vk().CreateImage(device->native(), &imCreateInfo, nullptr, &image));

    VkMemoryRequirements memReqs{};
    device->vk().GetImageMemoryRequirements(device->native(), image, &memReqs);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = device->gpu().memTypeIndex(memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    POLYPASSERTNOTEQUAL(memAllocInfo.memoryTypeIndex, UINT32_MAX);

    CHECKRET(device->vk().AllocateMemory(device->native(), &memAllocInfo, nullptr, &memory));
    CHECKRET(device->vk().BindImageMemory(device->native(), image, memory, 0));

    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.image = image;
    viewCreateInfo.format = imCreateInfo.format;
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;
    viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (viewCreateInfo.format >= VK_FORMAT_D16_UNORM_S8_UINT) {
        viewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;
    viewCreateInfo.image = image;

    CHECKRET(device->vk().CreateImageView(device->native(), &viewCreateInfo, nullptr, &view));

    return std::make_tuple(memory, image, view);
}

auto createRenderPass(Swapchain::ConstPtr swapchain) {
    auto device = swapchain->device();
    
    std::array<VkAttachmentDescription, 2> attachments = {};
    // Color attachment
    attachments[0].format         = swapchain->info().format.format;
    attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // Depth attachment
    attachments[1].format         = depthFormat(device);
    attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;

    // Subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    VkRenderPass renderPass = VK_NULL_HANDLE;
    CHECKRET(device->vk().CreateRenderPass(device->native(), &renderPassInfo, nullptr, &renderPass));

    return renderPass;
}

/// Creates a frame buffer for every image in the swapchain
std::vector<DESTROYABLE(VkFramebuffer)> createFrameBuffer(Swapchain::ConstPtr swapchain, 
                                                          VkImageView depthStencilView,
                                                          VkRenderPass renderPass) {
    auto device = swapchain->device();
    
    auto views = swapchain->views();
    std::vector<DESTROYABLE(VkFramebuffer)> frameBuffers(views.size());

    for (size_t i = 0; i < frameBuffers.size(); ++i) {
        std::array<VkImageView, 2> attachments;
        attachments[0] = views[i];
        attachments[1] = depthStencilView;

        VkFramebufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass = renderPass;
        createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        createInfo.pAttachments = attachments.data();
        createInfo.width  = swapchain->width();
        createInfo.height = swapchain->height();;
        createInfo.layers = 1;

        VkFramebuffer frameBuffer = VK_NULL_HANDLE;
        device->vk().CreateFramebuffer(device->native(), &createInfo, nullptr, &frameBuffer);

        POLYPDEBUG("Created VkFrameBuffer %p", frameBuffer);

        frameBuffers[i] = { frameBuffer, device };
    }

    return frameBuffers;
}

} // anonimus namespace

namespace polyp {
namespace engine {
namespace example {

void ExampleBase::preDraw() {
    auto [im, imIdx] = mSwapchain->aquireNextImage();
    if (im == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to get Swapchain images");
    }

    currSwImIndex = imIdx;

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CHECKRET(mDevice->vk().BeginCommandBuffer(mCmdBuffer, &beginInfo));

    currSwImBarrier.image = im;
    currSwImBarrier.srcAccessMask = VK_ACCESS_NONE;
    currSwImBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    currSwImBarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    currSwImBarrier.newLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    mDevice->vk().CmdPipelineBarrier(mCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &currSwImBarrier);
}

void ExampleBase::postDraw() {
    currSwImBarrier.srcAccessMask = currSwImBarrier.dstAccessMask;
    currSwImBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    currSwImBarrier.oldLayout     = /*VK_IMAGE_LAYOUT_UNDEFINED;*/currSwImBarrier.newLayout;
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

    CHECKRET(mDevice->vk().WaitForFences(mDevice->native(), 1, mSubmitFence.pNative(), VK_TRUE, kFenceTimeout));
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

    onResize();

    return true;
}

bool ExampleBase::onResize() {
    POLYPDEBUG(__FUNCTION__);

    auto res = mSwapchain->update();
    auto [memory, image, view] = createDepthStencil(mDevice, mSwapchain->surface());
    mDepthStencil = ImageResource{ {memory, mDevice}, {image, mDevice}, {view, mDevice} };
    mRenderPass = { createRenderPass(mSwapchain), mDevice };
    mFrameBuffers = createFrameBuffer(mSwapchain, *mDepthStencil.view, *mRenderPass);
    
    return res;
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
