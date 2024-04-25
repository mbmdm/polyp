#include "example_raii.h"

//#include <vulkan/vulkan_core.h>

//#include "windows.h"
//#include <vulkan/vulkan.h>
//#include <vulkan/vulkan_win32.h>

#include "vk_utils.h"
#include "vk_stringise.h"

//using namespace ::vk::raii;

using namespace vk::raii;
using polyp::vulkan::RHIContext;

namespace {

using namespace polyp::constants;

auto depthFormat(const PhysicalDevice& physDevice) {

    std::vector<vk::Format> dsDesiredFormats = {
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD32Sfloat,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD16UnormS8Uint,
        vk::Format::eD16Unorm
    };

    auto depthFormat = vk::Format::eUndefined;
    for (const auto& format : dsDesiredFormats) {
        auto props = physDevice.getFormatProperties(format);
        if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            depthFormat = format;
            break;
        }
    }
    POLYPASSERTNOTEQUAL(depthFormat, vk::Format::eUndefined);

    return depthFormat;
}

uint32_t memTypeIndex(const vk::PhysicalDeviceMemoryProperties& memProps, 
                      const vk::MemoryRequirements& memReq, 
                      vk::MemoryPropertyFlags flags) {
    auto typeBits = memReq.memoryTypeBits;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((typeBits & 1) == 1) {
            if ((memProps.memoryTypes[i].propertyFlags & flags) == flags) {
                return i;
            }
        }
        typeBits >>= 1;
    }
    return UINT32_MAX;
}

auto createDepthStencil(const Device& device, const PhysicalDevice& physDevice, const SurfaceKHR& surface) {

    Image image = VK_NULL_HANDLE;
    ImageView view = VK_NULL_HANDLE;
    DeviceMemory memory = VK_NULL_HANDLE;


    auto capabilities = physDevice.getSurfaceCapabilitiesKHR(*surface);
    const auto width = capabilities.currentExtent.width;
    const auto height = capabilities.currentExtent.height;

    vk::ImageCreateInfo imCreateInfo{};
    imCreateInfo.imageType = vk::ImageType::e2D;
    imCreateInfo.format = depthFormat(physDevice);
    imCreateInfo.extent = vk::Extent3D(width, height, 1);
    imCreateInfo.mipLevels = 1;
    imCreateInfo.arrayLayers = 1;
    imCreateInfo.samples = vk::SampleCountFlagBits::e1;
    imCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imCreateInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;

    image = device.createImage(imCreateInfo);

    auto memReqs = image.getMemoryRequirements();
    auto memProps = physDevice.getMemoryProperties();

    vk::MemoryAllocateInfo memAllocInfo{};
    memAllocInfo.allocationSize = memReqs.size;
    memAllocInfo.memoryTypeIndex = memTypeIndex(memProps, memReqs, vk::MemoryPropertyFlagBits::eDeviceLocal);
    memory = device.allocateMemory(memAllocInfo);

    image.bindMemory(*memory, 0);

    vk::ImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.viewType = vk::ImageViewType::e2D;
    viewCreateInfo.image = *image;
    viewCreateInfo.format = imCreateInfo.format;
    viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    if (viewCreateInfo.format >= vk::Format::eD16UnormS8Uint) {
        viewCreateInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
    }
    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = 1;

    view = device.createImageView(viewCreateInfo);

    return std::make_tuple(std::move(memory), std::move(image), std::move(view));
}

auto createRenderPass(const SwapchainKHR &swapchain, const PhysicalDevice& physDevice, const Device& device) {

    std::array<vk::AttachmentDescription, 2> attachments = {};
    // Color attachment
    attachments[0].format         = vk::Format::eR8G8B8A8Unorm;
    attachments[0].samples        = vk::SampleCountFlagBits::e1;
    attachments[0].loadOp         = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp        = vk::AttachmentStoreOp::eStore;
    attachments[0].stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].initialLayout  = vk::ImageLayout::eUndefined;
    attachments[0].finalLayout    = vk::ImageLayout::ePresentSrcKHR;
    // Depth attachment
    attachments[1].format         = depthFormat(physDevice);
    attachments[1].samples        = vk::SampleCountFlagBits::e1;
    attachments[1].loadOp         = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp        = vk::AttachmentStoreOp::eStore;
    attachments[1].stencilLoadOp  = vk::AttachmentLoadOp::eClear;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].initialLayout  = vk::ImageLayout::eUndefined;
    attachments[1].finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    subpassDescription.pResolveAttachments = nullptr;

    std::array<vk::SubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
    dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
    dependencies[0].srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    dependencies[0].dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
    dependencies[0].dependencyFlags = {};

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[1].srcAccessMask = vk::AccessFlags();
    dependencies[1].dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
    dependencies[0].dependencyFlags = {};

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();

    return device.createRenderPass(renderPassInfo);
}

VkDeviceSize memory(const ::vk::raii::PhysicalDevice& device) {

    VkDeviceSize output = 0;

    ::vk::PhysicalDeviceMemoryProperties memProperties = device.getMemoryProperties();

    std::vector<size_t> targetHeapsIdx;
    for (size_t heapTypeIdx = 0; heapTypeIdx < memProperties.memoryTypeCount; ++heapTypeIdx) {
        if (memProperties.memoryTypes[heapTypeIdx].propertyFlags & ::vk::MemoryPropertyFlagBits::eDeviceLocal) {
            targetHeapsIdx.push_back(memProperties.memoryTypes[heapTypeIdx].heapIndex);
        }
    }

    if (targetHeapsIdx.empty()) {
        return output;
    }

    //remove the same indexes if exist
    std::sort(targetHeapsIdx.begin(), targetHeapsIdx.end(), std::less<size_t>());
    auto currItr = targetHeapsIdx.begin() + 1;
    while (currItr != targetHeapsIdx.end()) {
        if (*currItr == *(currItr - 1)) {
            currItr = targetHeapsIdx.erase(currItr);
        }
        else {
            currItr++;
        }
    }

    for (size_t i = 0; i < targetHeapsIdx.size(); i++) {
        output += memProperties.memoryHeaps[targetHeapsIdx[i]].size;
    }

    return output;
}

bool isDiscrete(const ::vk::raii::PhysicalDevice& device)
{
    ::vk::PhysicalDeviceProperties props = device.getProperties();
    return props.deviceType == ::vk::PhysicalDeviceType::eDiscreteGpu;
}

std::string name(const ::vk::raii::PhysicalDevice& device)
{
    ::vk::PhysicalDeviceProperties props = device.getProperties();
    return props.deviceName;
}

std::vector<bool> checkSupport(const ::vk::raii::PhysicalDevice& device, ::vk::QueueFlags flags, uint32_t count)
{
    auto queFamilyProps = device.getQueueFamilyProperties();
    std::vector<bool> output(queFamilyProps.size(), false);

    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;

    for (uint32_t i = 0; i < output.size(); i++)
    {
        if ((queFamilyProps[i].queueFlags & flags) &&
            (queFamilyProps[i].queueCount >= count))
        {
            output[i] = true;
        }
    }

    return output;
}


std::vector<bool> checkSupport(const SurfaceKHR& surface, const PhysicalDevice& device) {

    auto queFamilyProps = device.getQueueFamilyProperties();

    std::vector<bool> output(queFamilyProps.size());
    for (size_t i = 0; i < output.size(); i++)
    {
        output[i] = device.getSurfaceSupportKHR(i, *surface) == VK_TRUE;
    }
    return output;
}

bool checkSupport(const ::vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface, vk::SwapchainCreateInfoKHR& info)
{
    auto capabilities = device.getSurfaceCapabilitiesKHR(*surface);
    if (capabilities.currentExtent.width == UINT32_MAX ||
        capabilities.currentExtent.height == UINT32_MAX) {
        return false;
    }
    else if (capabilities.currentExtent.width == 0 ||
        capabilities.currentExtent.height == 0) {
        return false;
    }

    bool marker = false;
    auto presentModes = device.getSurfacePresentModesKHR(*surface);
    for (size_t i = 0; i < presentModes.size(); ++i) {
        if (presentModes[i] == info.presentMode)
            marker = true;
    }
    if (!marker && !presentModes.empty())
    {
        POLYPWARN("Requested presentation %s mode is not found. Will be used %s", vk::to_string(info.presentMode).c_str(), vk::to_string(presentModes[0]).c_str());
        info.presentMode = presentModes[0];
        marker = true;
    }
    if (!marker)
        return false;




    return true;
}

bool checkSupport(const ::vk::raii::PhysicalDevice& device, const vk::raii::SurfaceKHR& surface)
{
    auto capabilities = device.getSurfaceCapabilitiesKHR(*surface);
    if (capabilities.currentExtent.width == UINT32_MAX ||
        capabilities.currentExtent.height == UINT32_MAX) {
        return false;
    }
    else if (capabilities.currentExtent.width == 0 ||
        capabilities.currentExtent.height == 0) {
        return false;
    }
    return true;
}

bool checkSupport(const PhysicalDevice& device, const SurfaceKHR& surface, vk::PresentModeKHR &mode)
{
    auto presentModes = device.getSurfacePresentModesKHR(*surface);
    for (size_t i = 0; i < presentModes.size(); ++i) {
        presentModes[i] = mode;
        return true;
    }

    if (presentModes.empty())
        return false;

    mode = presentModes[0];
}

bool update(SwapchainKHR& swapchain, const SurfaceKHR& surface, const PhysicalDevice& device, const Device& logicDevice)
{
    vk::SwapchainCreateInfoKHR swCreateInfo{};
    swCreateInfo.surface = *surface;
    swCreateInfo.minImageCount = 3;  // Double buffering
    swCreateInfo.imageFormat = vk::Format::eR8G8B8A8Unorm; // Commonly supported format
    swCreateInfo.imageColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;; // Standard color space
    swCreateInfo.presentMode = vk::PresentModeKHR::eMailbox;
    swCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment; // Use images as color attachments
    swCreateInfo.imageExtent = device.getSurfaceCapabilitiesKHR(*surface).currentExtent;
    swCreateInfo.imageArrayLayers = 1; // Single layer, no stereoscopic-3D
    swCreateInfo.imageSharingMode = vk::SharingMode::eExclusive; // Image is owned by one queue family at a time
    swCreateInfo.preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    swCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swCreateInfo.clipped = true; // Enable clipping
    swCreateInfo.oldSwapchain = *swapchain;

    if (!checkSupport(device, surface, swCreateInfo))
        return false;

    swapchain = logicDevice.createSwapchainKHR(swCreateInfo);
    if (*swapchain == VK_NULL_HANDLE)
        return false;

    return true;
}

} // anonimus namespace

namespace polyp {
namespace example {

void ExampleBaseRAII::preDraw() {

    auto [res, imIdx] = mSwapchain.acquireNextImage(constants::kFenceTimeout, VK_NULL_HANDLE, *mAqImageFence);
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

void ExampleBaseRAII::postDraw() {
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
    presentInfo.pSwapchains = &*mSwapchain;
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

bool ExampleBaseRAII::onInit(WindowInstance inst, WindowHandle hwnd) {
    POLYPDEBUG(__FUNCTION__);

    auto& ctx = vulkan::RHIContext::get();

    ctx.init({ {__FILE__, 1}, RHIContext::CreateInfo::GPU::Powerful, {inst, hwnd}, {1, vk::QueueFlagBits::eGraphics} });

    if (!ctx.ready())
    {
        POLYPFATAL("Failed to initialize Vulkan infrastructure.");
        return false;
    }

    POLYPINFO("Device %s will be used", vulkan::to_string(ctx.gpu()).c_str());

    const auto& device = ctx.device();

    mQueue = ctx.queue(0);
    if (*mQueue == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create vulkan graphics queue");
    }
    POLYPINFO("Graphics queue retrieved successfully");

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = 0 /*queueCreateInfo.queueFamilyIndex*/;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    mCmdPool = device.createCommandPool(poolInfo);
    if (*mCmdPool == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create command pool");
    }

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *mCmdPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

   auto cmds = device.allocateCommandBuffers(allocInfo);
    if (cmds.empty()) {
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
    mSubmitFence = device.createFence(fenceCreateInfo);
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

    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;


    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
    allocatorCreateInfo.physicalDevice = static_cast<VkPhysicalDevice>(*ctx.gpu());
    allocatorCreateInfo.device = static_cast<VkDevice>(*device);
    allocatorCreateInfo.instance = static_cast<VkInstance>(*ctx.instance());;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    
    auto vkres = vk::Result(vmaCreateAllocator(&allocatorCreateInfo, &mVmaAllocator));
    if (vkres != vk::Result::eSuccess) {
        POLYPFATAL("Failed to create VMA allocator.");
    }

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = 65536;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VkBuffer buffer;
    VmaAllocation allocation;
    vkres = vk::Result(vmaCreateBuffer(mVmaAllocator, &bufferInfo, &vmaAllocInfo, &buffer, &allocation, nullptr));
    if (vkres != vk::Result::eSuccess) {
        POLYPFATAL("Failed to create VkBuffer through VMA.");
    }

    return true;
}

bool ExampleBaseRAII::onResize() {
    POLYPDEBUG(__FUNCTION__);

    auto& ctx = vulkan::RHIContext::get();

    vulkan::RHIContext::get().device().waitIdle();
    if (!update(mSwapchain, ctx.surface(), vulkan::RHIContext::get().gpu(), vulkan::RHIContext::get().device()))
        POLYPFATAL("Failed to create vulkan swap chain.");

    auto [memory, image, view] = createDepthStencil(vulkan::RHIContext::get().device(), vulkan::RHIContext::get().gpu(), ctx.surface());
    mDepthStencil = ImageResource{ std::move(memory), std::move(image), std::move(view) };
    mRenderPass   = createRenderPass(mSwapchain, vulkan::RHIContext::get().gpu(), vulkan::RHIContext::get().device());

    mSwapChainImages = mSwapchain.getImages();
    mSwapChainVeiews.clear();
    mFrameBuffers.clear();
    {
        for (auto i = 0; i < mSwapChainImages.size(); ++i) {
            vk::ImageViewCreateInfo viewCreateInfo{};
            viewCreateInfo.format = vk::Format::eR8G8B8A8Unorm;
            viewCreateInfo.components = {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A
            };
            viewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            viewCreateInfo.subresourceRange.baseMipLevel = 0;
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.subresourceRange.baseArrayLayer = 0;
            viewCreateInfo.subresourceRange.layerCount = 1;
            viewCreateInfo.viewType = vk::ImageViewType::e2D;
            viewCreateInfo.flags = {};
            viewCreateInfo.image = mSwapChainImages[i];

            auto view = vulkan::RHIContext::get().device().createImageView(viewCreateInfo);
            mSwapChainVeiews.push_back(std::move(view));

            std::array<vk::ImageView, 2> attachments;
            attachments[0] = *mSwapChainVeiews[i];
            attachments[1] = *mDepthStencil.view;

            auto capabilities = vulkan::RHIContext::get().gpu().getSurfaceCapabilitiesKHR(*ctx.surface());

            vk::FramebufferCreateInfo fbCreateInfo{};
            fbCreateInfo.renderPass = *mRenderPass;
            fbCreateInfo.attachmentCount = attachments.size();
            fbCreateInfo.pAttachments = attachments.data();
            fbCreateInfo.width = capabilities.currentExtent.width;
            fbCreateInfo.height = capabilities.currentExtent.height;
            fbCreateInfo.layers = 1;

            auto fb = vulkan::RHIContext::get().device().createFramebuffer(fbCreateInfo);
            mFrameBuffers.push_back(std::move(fb));
        }
    }

    return true;
}

void ExampleBaseRAII::draw() {
    preDraw();
    POLYPDEBUG(__FUNCTION__);
    postDraw();
}

void ExampleBaseRAII::onShoutDown() {
    POLYPDEBUG(__FUNCTION__);
    vulkan::RHIContext::get().device().waitIdle();
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
} // polyp
