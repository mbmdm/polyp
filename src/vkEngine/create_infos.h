#ifndef CREATE_INFOS_H
#define CREATE_INFOS_H

#include "dispatch_table.h"

#include <constants.h>

#include <vector>
#include <string>

namespace polyp {
namespace vk {

struct InstanceCreateInfo {
    uint32_t mMajorVersion = constants::kMajorVersion;
    uint32_t mMinorVersion = constants::kMinorVersion;
    uint32_t mPatchVersion = constants::kPatchVersion;
    std::vector<const char*> mDesiredExtentions{ VK_KHR_SURFACE_EXTENSION_NAME,
                                                 VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    std::string              mApplicationName = constants::kInternalApplicationName;
};

struct QueueCreateInfo {
    uint32_t           mFamilyIndex = UINT32_MAX;
    VkQueueFlags       mQueueType = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    std::vector<float> mPriorities{ 1.0 };
};

struct DeviceCreateInfo {
    std::vector<QueueCreateInfo> mQueueInfo{};
    std::vector<const char*>     mDesiredExtentions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

struct SurfaceCreateInfo {
    polyp::tools::WindowInstance mWindowsInstance;
    polyp::tools::WindowHandle   mWindowsHandle;
};

struct SwapChainCreateInfo {
    VkPresentModeKHR presentationMode            = VK_PRESENT_MODE_FIFO_KHR;
    VkImageUsageFlagBits usage                   = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSurfaceTransformFlagBitsKHR transformation = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkSurfaceFormatKHR format                    = { VK_FORMAT_R8G8B8A8_UNORM, 
                                                     VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
};

} // engine
} // polyp

#endif // CREATE_INFOS_H
