#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "common.h"
#include "destroyer.h"
#include "device.h"
#include "surface.h"

namespace polyp {
namespace engine {

struct SwapChainCreateInfo {
};

/// Vulkan engin swapchain.
class Swapchain final {
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
};

} // engine
} // polyp

#endif // SWAPCHAIN_H
