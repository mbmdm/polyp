#include "swapchain.h"

namespace polyp {
namespace engine {

namespace {

} // anonymous namespace

Swapchain::Swapchain(Device::Ptr device, Surface::Ptr surface) :
                     mInfo{}, mDevice{ device }, mSurface{surface}
{
}

Swapchain::Swapchain(Device::Ptr device, Surface::Ptr surface, const SwapChainCreateInfo& info) : 
                     Swapchain(device, surface) 
{
    mInfo = info;
}

VkSwapchainKHR const& Swapchain::operator*() const
{
    // TODO: insert return statement here
    return VK_NULL_HANDLE;
}

bool Swapchain::init()
{
    return false;
}

} // engine
} // polyp
