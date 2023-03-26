#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "destroyer.h"
#include "device.h"
#include "surface.h"

namespace polyp {
namespace engine {

struct SwapChainCreateInfo {
    VkPresentModeKHR presentationMode            = VK_PRESENT_MODE_FIFO_KHR;
    VkImageUsageFlagBits usage                   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR transformation = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkSurfaceFormatKHR format                    = { VK_FORMAT_R8G8B8A8_UNORM, 
                                                     VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
};

/// Vulkan engin swapchain.
class Swapchain final : public std::enable_shared_from_this<Swapchain> {
private:
    Swapchain(Device::Ptr device, Surface::Ptr surface);
    Swapchain(Device::Ptr device, Surface::Ptr surface, const SwapChainCreateInfo& info);

public:
    using Ptr = std::shared_ptr<Swapchain>;

    Swapchain(const Swapchain&)            = delete;
    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain(Swapchain&&)                 = delete;
    Swapchain& operator=(Swapchain&&)      = delete;
    ~Swapchain()                           = default;

    /// Returns next swapchain Image or VK_NULL_HANDLE if failed
    /// Also VK_NULL_HANDLE will be returned on timeout
    [[nodiscard]] VkImage nextImage() const;
    /// Updates Swapchain according new SwapChainCreateInfo.
    /// Also can be used when Surface size was changed.
    [[nodiscard]] bool update();

    /// Creates swapchain
    /// 
    /// Typical usage:
    /// \code
    ///   create(Device::Ptr device, Surface::Ptr surface, const SwapChainCreateInfo& info);
    /// \endcode
    /// 
    /// \param device  - polyp::engine::Device::Ptr;
    /// \param surface - polyp::engine::Surface::Ptr;
    /// \param info    - additional swapchain creation info;
    template<typename ...Args>
    [[nodiscard]] static Ptr create(Args... args) {
        std::shared_ptr<Swapchain> output(new Swapchain(args...));
        if (!output->init()) {
            output.reset();
        }
        return output;
    }

    VkSwapchainKHR const& operator*() const;

private:
    [[nodiscard]] bool init();

    SwapChainCreateInfo                 mInfo;
    Device::Ptr                         mDevice;
    Surface::Ptr                        mSurface;
    DECLARE_VKDESTROYER(VkSwapchainKHR) mHandle;
    std::vector<VkImage>                mImages;
    DECLARE_VKDESTROYER(VkFence)        mFence;
};

} // engine
} // polyp

#endif // SWAPCHAIN_H
