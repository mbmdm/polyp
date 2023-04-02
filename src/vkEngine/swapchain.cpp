#include "swapchain.h"

#include <array>

namespace polyp {
namespace engine {

namespace {

/// Used when desired presentation mode is not supported. Fill this arrya according to priorities.
/// E.g., if mode [0] is supported than the exact this presentation mode will be returned in 
/// mostSuitablePresentationMode() function.
inline constexpr std::array<VkPresentModeKHR, 2> gSuitablePresentationModePriorities{
    VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR };
/// Default minimum number of images to be requested
inline constexpr uint32_t gMinSwapchainRequiestedImagesCount = 3;
/// Default timeout for swapchain image acquiring
inline constexpr uint32_t gImageAcquireTimeoutNs = 2000000000;

/// Returns the best available presentation mode. Returns VK_PRESENT_MODE_MAX_ENUM_KHR if failed
VkPresentModeKHR mostSuitablePresentationMode(Surface::Ptr surface, const PhysicalGpu& gpu) {
    auto modes = surface->presentModes(gpu);
    std::vector<bool> seekResult(gSuitablePresentationModePriorities.size(), false);
    for (size_t i = 0; i < modes.size(); i++) {
        auto seekResultIt = std::find(gSuitablePresentationModePriorities.begin(), 
                                      gSuitablePresentationModePriorities.end(), 
                                      modes[i]);
        if (seekResultIt != gSuitablePresentationModePriorities.end()) {
            seekResult[seekResultIt - gSuitablePresentationModePriorities.begin()] = true;
        }
    }
    for (size_t i = 0; i < seekResult.size(); i++) {
        if (seekResult[i]) {
            return gSuitablePresentationModePriorities[i];
        }
    }
    return VK_PRESENT_MODE_MAX_ENUM_KHR;
}

/// Returns image handles of created swapchain
std::vector<VkImage> getSwapchainImages(Device::Ptr device, Swapchain::Ptr swapchain) {
    uint32_t count = 0;
    VkResult result = VK_SUCCESS;
    CHECKRET(device->vk().GetSwapchainImagesKHR(**device, **swapchain, &count, nullptr));
    std::vector<VkImage> output(count);
    CHECKRET(device->vk().GetSwapchainImagesKHR(**device, **swapchain, &count, output.data()));
    return output;
}

/// Creates a new VkFence in signaled state
VkFence createFence(Device::Ptr device) {
    VkFence fence = VK_NULL_HANDLE;
    VkFenceCreateInfo createInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    createInfo.pNext = nullptr;
    //createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    CHECKRET(device->vk().CreateFence(**device, &createInfo, nullptr, &fence));
    return fence;
}

} // anonymous namespace

Swapchain::Swapchain(Device::Ptr device, Surface::Ptr surface) :
                     mInfo{}, mDevice{ device }, mSurface{ surface }, 
                     mHandle{ {**device, VK_NULL_HANDLE}, nullptr },
                     mFence{ {**device, VK_NULL_HANDLE}, nullptr }
{
}

Swapchain::Swapchain(Device::Ptr device, Surface::Ptr surface, const SwapChainCreateInfo& info) : 
                     Swapchain(device, surface) 
{
    mInfo = info;
}

std::tuple<VkImage, uint32_t> Swapchain::nextImage() const {
    uint32_t imIndex = 0;
    CHECKRET(mDevice->vk().AcquireNextImageKHR(**mDevice, *mHandle, gImageAcquireTimeoutNs, VK_NULL_HANDLE, *mFence, &imIndex));
    CHECKRET(mDevice->vk().WaitForFences(**mDevice, 1, &*mFence, VK_TRUE, gImageAcquireTimeoutNs));
    CHECKRET(mDevice->vk().GetFenceStatus(**mDevice, *mFence));
    CHECKRET(mDevice->vk().ResetFences(**mDevice, 1, &*mFence));
    POLYPDEBUG("Returned swapchain image idx %d", imIndex);
    return std::make_tuple(mImages[imIndex], imIndex);
}

bool Swapchain::update() {
    mDevice->vk().DeviceWaitIdle(**mDevice);
    DECLARE_VKDESTROYER(VkSwapchainKHR) oldHandle{ {**mDevice, VK_NULL_HANDLE}, nullptr };
    oldHandle = std::move(mHandle); //swap
    return init(*oldHandle);
}

VkSwapchainKHR const& Swapchain::operator*() const {
    return *mHandle;
}

bool Swapchain::init(VkSwapchainKHR oldSwapChain) {
    // check presentation mode
    auto isSupported = mSurface->checkSupport(mDevice->gpu(), mInfo.presentationMode);
    if (!isSupported) {
        POLYPWARN("Failed to select desired presentation mode");
        mInfo.presentationMode = mostSuitablePresentationMode(mSurface, mDevice->gpu());
        POLYPWARN("Selected presentation mode %d", mInfo.presentationMode);
    }

    // check current VkExtent2D
    auto surfaceCapabilities = mSurface->capabilities(mDevice->gpu());
    if (surfaceCapabilities.currentExtent.width == UINT32_MAX ||
        surfaceCapabilities.currentExtent.height == UINT32_MAX) {
        POLYPWARN("Unsupported swapchain creation scenario.");
        return false;
    }
    else if (surfaceCapabilities.currentExtent.width == 0 ||
             surfaceCapabilities.currentExtent.height == 0) {
        POLYPWARN("Failed to create swapchain, maybe the window was minimized. Try again later.");
        return false;
    }

    /// tune number of swapchain images
    auto numberOfImages = std::max(surfaceCapabilities.minImageCount + 1, 
                                   gMinSwapchainRequiestedImagesCount);
    if (surfaceCapabilities.maxImageCount != 0) { // if there's a limit ?
        numberOfImages = std::min(numberOfImages, surfaceCapabilities.maxImageCount);
    }

    // check swapchain usage
    if ((surfaceCapabilities.supportedUsageFlags & mInfo.usage) != mInfo.usage) {
        POLYPWARN("Failed to create swapchain with required usage flags");
        return false;
    }

    // check image pixel transformation
    if ((surfaceCapabilities.supportedTransforms & mInfo.transformation) == 0) {
        POLYPWARN("Surface doen't support the required swapchain image transformation");
        mInfo.transformation = surfaceCapabilities.currentTransform;
    }

    // check formats and color space
    auto surfaceFormats = mSurface->format(mDevice->gpu());
    if (!mSurface->checkSupport(mDevice->gpu(), mInfo.format)) {
        mSurface->checkSupport(mDevice->gpu(), mInfo.format/*in*/, mInfo.format/*out*/);
    }

    VkSwapchainCreateInfoKHR createInfo = {
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, // VkStructureType                 sType
    nullptr,                                     // const void                    * pNext
    0,                                           // VkSwapchainCreateFlagsKHR       flags
    **mSurface,                                  // VkSurfaceKHR                    surface
    numberOfImages,                              // uint32_t                        minImageCount
    mInfo.format.format,                         // VkFormat                        imageFormat
    mInfo.format.colorSpace,                     // VkColorSpaceKHR                 imageColorSpace
    surfaceCapabilities.currentExtent,           // VkExtent2D                      imageExtent
    1,                                           // uint32_t                        imageArrayLayers
    mInfo.usage,                                 // VkImageUsageFlags               imageUsage
    VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode                   imageSharingMode
    0,                                           // uint32_t                        queueFamilyIndexCount
    nullptr,                                     // const uint32_t                * pQueueFamilyIndices
    mInfo.transformation,                        // VkSurfaceTransformFlagBitsKHR   preTransform
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,           // VkCompositeAlphaFlagBitsKHR     compositeAlpha
    mInfo.presentationMode,                      // VkPresentModeKHR                presentMode
    VK_TRUE,                                     // VkBool32                        clipped
    oldSwapChain                                 // VkSwapchainKHR                  oldSwapchain
    };

    CHECKRET(mDevice->vk().CreateSwapchainKHR(**mDevice, &createInfo, nullptr, &*mHandle));
    if (*mHandle == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create swapchain.");
    }

    initVkDestroyer(mDevice->vk().DestroySwapchainKHR, mHandle, nullptr);

    mImages.clear(); // just explicit call
    mImages = getSwapchainImages(mDevice, shared_from_this());

    if (oldSwapChain == VK_NULL_HANDLE) { // comes from Swapchan::update
        *mFence = createFence(mDevice);
        initVkDestroyer(mDevice->vk().DestroyFence, mFence, nullptr);
    }

    return true;
}

} // engine
} // polyp
