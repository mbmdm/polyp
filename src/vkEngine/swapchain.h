#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "destroyer.h"
#include "device.h"
#include "surface.h"

namespace polyp {
namespace engine {

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

    Device::ConstPtr device()   const { return mDevice; };
    Surface::ConstPtr surface() const { return mSurface; };

    /// Returns swapchin create info
    SwapChainCreateInfo info()  const;

    /// Returns next swapchain Image and its index.
    /// Throws std::out_of_range if failed
    /// \param get<0> - VkImage handle
    /// \param get<1> - swapchain image index
    std::tuple<VkImage, uint32_t> aquireNextImage() const;
    /// Updates Swapchain according new SwapChainCreateInfo.
    /// Also can be used when Surface size was changed.
    bool update();
    /// Returns swapchain images count.
    size_t imageCount()              const;
    /// Returns swapchain images color format.
    VkFormat colorFormat()           const;
    /// Returns VkImageVeiw array for each swapchain image
    std::vector<VkImageView> views() const;
    /// Returns current surface width
    uint32_t width()  const;
    /// Returns current surface width
    uint32_t height() const;

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
    VkSwapchainKHR         native()   const;
    const VkSwapchainKHR* pNative()   const;

private:
    bool init(VkSwapchainKHR oldSwapChain);

    SwapChainCreateInfo         mInfo;
    Device::Ptr                 mDevice;
    Surface::Ptr                mSurface;
    DESTROYABLE(VkSwapchainKHR) mHandle;
    DESTROYABLE(VkFence)        mFence;

    std::vector<VkImage>                  mImages;
    std::vector<DESTROYABLE(VkImageView)> mImageViews;
};

} // engine
} // polyp

#endif // SWAPCHAIN_H
