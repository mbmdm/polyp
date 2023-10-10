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
    using Ptr      = std::shared_ptr<Swapchain>;
    using ConstPtr = std::shared_ptr<Swapchain>;

    Swapchain(const Swapchain&)            = delete;
    Swapchain& operator=(const Swapchain&) = delete;
    Swapchain(Swapchain&&)                 = delete;
    Swapchain& operator=(Swapchain&&)      = delete;
    ~Swapchain()                           = default;

    /// Returns next swapchain Image and its index.
    /// Throws std::out_of_range if failed
    /// \param get<0> - VkImage handle
    /// \param get<1> - swapchain image index
    [[nodiscard]] std::tuple<VkImage, uint32_t> nextImage() const;
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
        if (!output->init(VK_NULL_HANDLE)) {
            output.reset();
        }
        return output;
    }

    /// Returns underlying vulkan handle.
    VkSwapchainKHR const& operator*() const;
    /// Returns underlying vulkan handle.
    VkSwapchainKHR native()           const;

private:
    bool init(VkSwapchainKHR oldSwapChain);

    SwapChainCreateInfo         mInfo;
    Device::Ptr                 mDevice;
    Surface::Ptr                mSurface;
    DESTROYABLE(VkSwapchainKHR) mHandle;
    std::vector<VkImage>        mImages;
    DESTROYABLE(VkFence)        mFence;
};

} // engine
} // polyp

#endif // SWAPCHAIN_H
