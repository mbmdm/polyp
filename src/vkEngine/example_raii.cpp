#include "example_raii.h"

//#include <vulkan/vulkan_core.h>

//#include "windows.h"
//#include <vulkan/vulkan.h>
//#include <vulkan/vulkan_win32.h>

//using namespace ::vk::raii;

using namespace vk::raii;

namespace {

using namespace polyp::constants;

auto depthFormat(vk::raii::Device device) {
    //std::vector<VkFormat> dsDesiredFormats = {
    //VK_FORMAT_D32_SFLOAT_S8_UINT,
    //VK_FORMAT_D32_SFLOAT,
    //VK_FORMAT_D24_UNORM_S8_UINT,
    //VK_FORMAT_D16_UNORM_S8_UINT,
    //VK_FORMAT_D16_UNORM
    //};

    //VkFormat depthFormat = VK_FORMAT_UNDEFINED;
    //for (const auto& format : dsDesiredFormats) {
    //    VkFormatProperties formatProps;
    //    auto instance = device->instance();
    //    instance->vk().GetPhysicalDeviceFormatProperties(*device->gpu(), format, &formatProps);
    //    if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
    //        depthFormat = format;
    //        break;
    //    }
    //}
    //POLYPASSERT(depthFormat != VK_FORMAT_UNDEFINED);

    //return depthFormat;
    return 1;
}

auto createDepthStencil(vk::raii::Device device, vk::raii::SurfaceKHR surface) {
    //VkImage image         = VK_NULL_HANDLE;
    //VkImageView view      = VK_NULL_HANDLE;
    //VkDeviceMemory memory = VK_NULL_HANDLE;

    //const auto width  = surface->width(device->gpu());
    //const auto height = surface->height(device->gpu());

    //VkImageCreateInfo imCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    //imCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    //imCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    //imCreateInfo.format = depthFormat(device);
    //imCreateInfo.extent = { width, height, 1 };
    //imCreateInfo.mipLevels = 1;
    //imCreateInfo.arrayLayers = 1;
    //imCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    //imCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    //imCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    //CHECKRET(device->vk().CreateImage(device->native(), &imCreateInfo, nullptr, &image));

    //VkMemoryRequirements memReqs{};
    //device->vk().GetImageMemoryRequirements(device->native(), image, &memReqs);

    //VkMemoryAllocateInfo memAllocInfo{};
    //memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    //memAllocInfo.allocationSize = memReqs.size;
    //memAllocInfo.memoryTypeIndex = device->gpu().memTypeIndex(memReqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    //POLYPASSERTNOTEQUAL(memAllocInfo.memoryTypeIndex, UINT32_MAX);

    //CHECKRET(device->vk().AllocateMemory(device->native(), &memAllocInfo, nullptr, &memory));
    //CHECKRET(device->vk().BindImageMemory(device->native(), image, memory, 0));

    //VkImageViewCreateInfo viewCreateInfo{};
    //viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    //viewCreateInfo.image = image;
    //viewCreateInfo.format = imCreateInfo.format;
    //viewCreateInfo.subresourceRange.baseMipLevel = 0;
    //viewCreateInfo.subresourceRange.levelCount = 1;
    //viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    //viewCreateInfo.subresourceRange.layerCount = 1;
    //viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    //if (viewCreateInfo.format >= VK_FORMAT_D16_UNORM_S8_UINT) {
    //    viewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    //}
    //viewCreateInfo.subresourceRange.baseMipLevel = 0;
    //viewCreateInfo.subresourceRange.levelCount = 1;
    //viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    //viewCreateInfo.subresourceRange.layerCount = 1;
    //viewCreateInfo.image = image;

    //CHECKRET(device->vk().CreateImageView(device->native(), &viewCreateInfo, nullptr, &view));

    //return std::make_tuple(memory, image, view);

    return 1;
}

auto createRenderPass(vk::raii::SwapchainKHR swapchain) {
    //auto device = swapchain->device();
    //
    //std::array<VkAttachmentDescription, 2> attachments = {};
    //// Color attachment
    //attachments[0].format         = swapchain->info().format.format;
    //attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
    //attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    //attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    //attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //// Depth attachment
    //attachments[1].format         = depthFormat(device);
    //attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
    //attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    //attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    //attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //VkAttachmentReference colorReference = {};
    //colorReference.attachment = 0;
    //colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //VkAttachmentReference depthReference = {};
    //depthReference.attachment = 1;
    //depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //VkSubpassDescription subpassDescription = {};
    //subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //subpassDescription.colorAttachmentCount = 1;
    //subpassDescription.pColorAttachments = &colorReference;
    //subpassDescription.pDepthStencilAttachment = &depthReference;
    //subpassDescription.inputAttachmentCount = 0;
    //subpassDescription.pInputAttachments = nullptr;
    //subpassDescription.preserveAttachmentCount = 0;
    //subpassDescription.pPreserveAttachments = nullptr;
    //subpassDescription.pResolveAttachments = nullptr;

    //// Subpass dependencies for layout transitions
    //std::array<VkSubpassDependency, 2> dependencies;

    //dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[0].dstSubpass = 0;
    //dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    //dependencies[0].dependencyFlags = 0;

    //dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    //dependencies[1].dstSubpass = 0;
    //dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //dependencies[1].srcAccessMask = 0;
    //dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    //dependencies[1].dependencyFlags = 0;

    //VkRenderPassCreateInfo renderPassInfo = {};
    //renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    //renderPassInfo.pAttachments = attachments.data();
    //renderPassInfo.subpassCount = 1;
    //renderPassInfo.pSubpasses = &subpassDescription;
    //renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    //renderPassInfo.pDependencies = dependencies.data();

    //VkRenderPass renderPass = VK_NULL_HANDLE;
    //CHECKRET(device->vk().CreateRenderPass(device->native(), &renderPassInfo, nullptr, &renderPass));

    //return renderPass;

    return 1;
}

///// Creates a frame buffer for every image in the swapchain
//std::vector<DestroyableHandle<VkFramebuffer>> createFrameBuffer(Swapchain::ConstPtr swapchain,
//                                                                VkImageView depthStencilView,
//                                                                VkRenderPass renderPass) {
//    //auto device = swapchain->device();
//    //
//    //auto views = swapchain->views();
//    //std::vector<DestroyableHandle<VkFramebuffer>> frameBuffers;
//
//    //for (size_t i = 0; i < views.size(); ++i) {
//    //    std::array<VkImageView, 2> attachments;
//    //    attachments[0] = views[i];
//    //    attachments[1] = depthStencilView;
//
//    //    VkFramebufferCreateInfo createInfo{};
//    //    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//    //    createInfo.renderPass = renderPass;
//    //    createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//    //    createInfo.pAttachments = attachments.data();
//    //    createInfo.width  = swapchain->width();
//    //    createInfo.height = swapchain->height();;
//    //    createInfo.layers = 1;
//
//    //    VkFramebuffer frameBuffer = VK_NULL_HANDLE;
//    //    device->vk().CreateFramebuffer(device->native(), &createInfo, nullptr, &frameBuffer);
//
//    //    POLYPDEBUG("Created VkFrameBuffer %p", frameBuffer);
//
//    //    frameBuffers.emplace_back(frameBuffer, device);
//    //}
//
//    //return frameBuffers;
//    return {};
//}

VkDeviceSize memory(const ::vk::raii::PhysicalDevice& device) {

    VkDeviceSize output = 0;

    ::vk::PhysicalDeviceMemoryProperties memProperties = device.getMemoryProperties();

    //const auto &test1 = memProperties.memoryHeapCount;
    //const auto& test2 = memProperties.memoryHeaps;

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
    swCreateInfo.imageFormat = vk::Format::eR8G8B8Unorm; // Commonly supported format
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
    //auto [res, imIdx] = mSwapchain.acquireNextImage(constants::kFenceTimeout);
    //if (res !=  ::vk::Result::eSuccess ) {
    //    POLYPFATAL("Failed to get Swapchain images");
    //}

    //mSwapchain.image

    //currSwImIndex = imIdx;

    ////VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    ////beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;;

    //::vk::CommandBufferBeginInfo beginInfo{ ::vk::CommandBufferUsageFlagBits::eOneTimeSubmit };

    //mCmdBuffer.begin(beginInfo);

    //currSwImBarrier.image = im;
    //currSwImBarrier.srcAccessMask = VK_ACCESS_NONE;
    //currSwImBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //currSwImBarrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    //currSwImBarrier.newLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //mDevice->vk().CmdPipelineBarrier(mCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
    //                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &currSwImBarrier);
}

void ExampleBaseRAII::postDraw() {
    //currSwImBarrier.srcAccessMask = currSwImBarrier.dstAccessMask;
    //currSwImBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    //currSwImBarrier.oldLayout     = /*VK_IMAGE_LAYOUT_UNDEFINED;*/currSwImBarrier.newLayout;
    //currSwImBarrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    //mDevice->vk().CmdPipelineBarrier(mCmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //                                 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &currSwImBarrier);

    //CHECKRET(mDevice->vk().EndCommandBuffer(mCmdBuffer));;

    //VkSubmitInfo submitIinfo = {
    //    VK_STRUCTURE_TYPE_SUBMIT_INFO, // VkStructureType                sType
    //    nullptr,                       // const void                   * pNext
    //    0,                             // uint32_t                       waitSemaphoreCount
    //    nullptr,                       // const VkSemaphore            * pWaitSemaphores
    //    nullptr,                       // const VkPipelineStageFlags   * pWaitDstStageMask
    //    1,                             // uint32_t                       commandBufferCount
    //    &mCmdBuffer,                   // const VkCommandBuffer        * pCommandBuffers
    //    1,                             // uint32_t                       signalSemaphoreCount
    //    &mReadyToPresent               // const VkSemaphore            * pSignalSemaphores
    //};
    //CHECKRET(mDevice->vk().QueueSubmit(mQueue, 1, &submitIinfo, *mSubmitFence));

    //VkPresentInfoKHR presentInfo = {
    //    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // VkStructureType          sType
    //    nullptr,                            // const void*              pNext
    //    1,                                  // uint32_t                 waitSemaphoreCount
    //    &mReadyToPresent,                   // const VkSemaphore      * pWaitSemaphores
    //    1,                                  // uint32_t                 swapchainCount
    //    mSwapchain->pNative(),              // const VkSwapchainKHR   * pSwapchains
    //    &currSwImIndex,                     // const uint32_t         * pImageIndices
    //    nullptr                             // VkResult*                pResults
    //};
    //CHECKRET(mDevice->vk().QueuePresentKHR(mQueue, &presentInfo));

    //CHECKRET(mDevice->vk().WaitForFences(mDevice->native(), 1, &mSubmitFence, VK_TRUE, kFenceTimeout));
    //CHECKRET(mDevice->vk().GetFenceStatus(mDevice->native(), *mSubmitFence));
    //CHECKRET(mDevice->vk().ResetFences(mDevice->native(), 1, &mSubmitFence));
}

bool ExampleBaseRAII::onInit(WindowInstance inst, WindowHandle hwnd) {
    POLYPDEBUG(__FUNCTION__);

    ::vk::ApplicationInfo applicationInfo(constants::kInternalApplicationName, 1, constants::kInternalApplicationName, 1, VK_API_VERSION_1_2);

    std::vector<const char*> extensions{ 
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

    ::vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, 0, nullptr, extensions.size(), extensions.data());

    mInstance = vk::raii::Instance(mContext, instanceCreateInfo);
    if (*mInstance == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create vulkan instance");
        return false;
    }

    auto gpus = mInstance.enumeratePhysicalDevices();

    if (gpus.empty()) {
        POLYPFATAL("Required GPU not found");
        return false;
    }

    mPhysDevice = gpus[0];

    auto test = isDiscrete(mPhysDevice);

    auto memSize = memory(mPhysDevice);

    for (size_t i = 1; i < gpus.size(); i++) {
        auto gpu = gpus[i];
        if (memory(gpu) > memory(mPhysDevice) && isDiscrete(gpu)) {
            mPhysDevice = gpu;
        }
    }

    POLYPINFO("Selected device %s with local memory %d Mb\n", name(mPhysDevice).c_str(), memory(mPhysDevice) / 1024 / 1024);

    vk::Win32SurfaceCreateInfoKHR surfaceInfo({}, inst, hwnd, nullptr);
    mSurface = mInstance.createWin32SurfaceKHR(surfaceInfo);

    if (*mSurface == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create vulkan surface (WSI)");
        return false;
    }


    vk::DeviceCreateInfo deviceCreateInfo{};
    vk::DeviceQueueCreateInfo queueCreateInfo{};

    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    
    std::vector<float> quePriorities = { 1./*, 0.5 */ };
    queueCreateInfo.pQueuePriorities = quePriorities.data();
    queueCreateInfo.queueCount = quePriorities.size();
    queueCreateInfo.queueFamilyIndex = UINT32_MAX;

    {
        auto queReqFlags = ::vk::QueueFlagBits::eGraphics | ::vk::QueueFlagBits::eCompute;

        auto availableGraphicsQueue = checkSupport(mPhysDevice, queReqFlags, quePriorities.size());
        auto availableWSIQueue = checkSupport(mSurface, mPhysDevice);

        for (uint32_t i = 0; i < availableGraphicsQueue.size(); i++) {
            if (availableGraphicsQueue[i] && availableWSIQueue[i]) {
                queueCreateInfo.queueFamilyIndex = i;
                break;
            }
        }
        if (queueCreateInfo.queueFamilyIndex == UINT32_MAX) {
            POLYPFATAL("Failed to find the reqired queue family");
            return false;
        }
    }

    std::vector<const char*> extansions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    deviceCreateInfo.ppEnabledExtensionNames = extansions.data();
    deviceCreateInfo.enabledExtensionCount = extansions.size();

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    mDevice = mPhysDevice.createDevice(deviceCreateInfo);
    if (*mDevice == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create vulkan logical device");
        return false;
    }
    POLYPINFO("Vulkan device created successfully");

    mQueue = mDevice.getQueue(queueCreateInfo.queueFamilyIndex, 0);
    if (*mQueue == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create vulkan graphics queue");
    }
    POLYPINFO("Graphics queue retrieved successfully");


    //vk::SwapchainCreateInfoKHR swCreateInfo{};
    //swCreateInfo.surface          = *mSurface;
    //swCreateInfo.minImageCount    = 3;  // Double buffering
    //swCreateInfo.imageFormat      = vk::Format::eR8G8B8Unorm; // Commonly supported format
    //swCreateInfo.imageColorSpace  = vk::ColorSpaceKHR::eSrgbNonlinear;; // Standard color space
    //swCreateInfo.presentMode      = vk::PresentModeKHR::eMailbox;
    //swCreateInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment; // Use images as color attachments
    //swCreateInfo.imageExtent      = physGpu.getSurfaceCapabilitiesKHR(*mSurface).currentExtent;
    //swCreateInfo.imageArrayLayers = 1; // Single layer, no stereoscopic-3D
    //swCreateInfo.imageSharingMode = vk::SharingMode::eExclusive; // Image is owned by one queue family at a time
    //swCreateInfo.preTransform     = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    //swCreateInfo.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    //swCreateInfo.clipped          = true ; // Enable clipping
    //swCreateInfo.oldSwapchain     = VK_NULL_HANDLE;

    //if (!checkSupport(physGpu, mSurface, swCreateInfo))
    //    POLYPFATAL("Failed to create vulkan swap chain.");

    if (!update(mSwapchain, mSurface, mPhysDevice, mDevice))
        POLYPFATAL("Failed to create vulkan swap chain.");

    //mSwapchain = mDevice.createSwapchainKHR(swCreateInfo);
    //if (*mSwapchain == VK_NULL_HANDLE) {
    //    POLYPFATAL("Failed to create vulkan swap chain.");
    //}
    POLYPINFO("Vulkan swap chain created successfully");

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.queueFamilyIndex = queueCreateInfo.queueFamilyIndex;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    mCmdPool = mDevice.createCommandPool(poolInfo);
    if (*mCmdPool == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create command pool");
    }

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = *mCmdPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

   auto cmds = mDevice.allocateCommandBuffers(allocInfo);
    if (cmds.empty()) {
        POLYPFATAL("Failed to allocate command buffers.");
        return false;
    }

    mCmdBuffer = std::move(cmds[0]);
    POLYPINFO("Primary command buffer created successfully");

    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    mReadyToPresent = mDevice.createSemaphore(semaphoreCreateInfo);
    if (*mReadyToPresent == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create image available semaphore.");
    }

    vk::FenceCreateInfo fenceCreateInfo{/*vk::FenceCreateFlagBits::eSignaled*/};
    mSubmitFence = mDevice.createFence(fenceCreateInfo);
    if (*mSubmitFence == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create in-flight fence.");
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


    printf("");

    onResize();

    return true;
}

bool ExampleBaseRAII::onResize() {
    POLYPDEBUG(__FUNCTION__);

    mDevice.waitIdle();
    if (!update(mSwapchain, mSurface, mPhysDevice, mDevice))
        POLYPFATAL("Failed to create vulkan swap chain.");



    //auto res = mSwapchain->update();
    //auto [memory, image, view] = createDepthStencil(mDevice, mSwapchain->surface());
    //mDepthStencil = ImageResource{ {memory, mDevice}, {image, mDevice}, {view, mDevice} };
    //mRenderPass = { createRenderPass(mSwapchain), mDevice };
    //mFrameBuffers = createFrameBuffer(mSwapchain, *mDepthStencil.view, *mRenderPass);
    //
    //return res;
    return true;
}

void ExampleBaseRAII::draw() {
    preDraw();
    POLYPDEBUG(__FUNCTION__);
    postDraw();
}

void ExampleBaseRAII::onShoutDown() {
    //POLYPDEBUG(__FUNCTION__);
    //mDevice->vk().DeviceWaitIdle(mDevice->native());
    //POLYPTODO(
    //    "Need association cmdBuffer and cmdPool to have an ability to release cmdBuffer with vkFreeCommandBuffers()"
    //    "Seems I should move such login in device class and someway point out that native handles shouldn't be "
    //    "desctoyed manually. I can also have logic of recreatiion objects with different flags"
    //);
    //POLYPTODO(
    //    "Command buffers allocated from a given pool are implicitly freed when we destroy a"
    //    "command pool. So when we want to destroy a pool, we don't need to separately free all"
    //    "command buffers allocated from it. ((c) VulkanCookBook)."
    //);
    //POLYPTODO("Seems I shouldn't descroy them, but it can be useful in recreation approach");
}

} // example
} // polyp
