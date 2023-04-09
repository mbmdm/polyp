#include "surface.h"

namespace polyp {
namespace engine {

Surface::Surface(Instance::Ptr instance, const SurfaceCreateInfo& info) : 
                 mInstance{ instance }, mInfo{ info }, mHandle{ } 
{
    mHandle.setRoot(mInstance->raw());
}

const VkSurfaceKHR& Surface::operator*() const {
    return *mHandle;
}

VkSurfaceKHR Surface::raw() const {
    return this->operator*();
}

std::vector<bool> Surface::checkSupport(PhysicalGpu gpu) const {
    std::vector<bool> output(gpu.queueFamilyCount());
    VkBool32 flag = VK_FALSE;
    for (size_t i = 0; i < output.size(); i++) {
        CHECKRET(mInstance->vk().GetPhysicalDeviceSurfaceSupportKHR(*gpu, i, this->raw(), &flag));
        output[i] = flag == VK_TRUE;
    }
    return output;
}

bool Surface::checkSupport(PhysicalGpu gpu, VkSurfaceFormatKHR format) const {
    auto currFormats = Surface::format(gpu);
    if (currFormats.size() == 1 &&
        currFormats.begin()->format == VK_FORMAT_UNDEFINED) {
        POLYPINFO("Sufrace returned VK_FORMAT_UNDEFINED. All formats are supported");
        return true;
    }
    for (const auto& currFormat : currFormats) {
        if (format.format == currFormat.format &&
            format.colorSpace == currFormat.colorSpace) {
            return true;
        }
    }
    return false;
}

bool Surface::checkSupport(PhysicalGpu gpu, VkSurfaceFormatKHR in_format,
                           VkSurfaceFormatKHR& out_format) const {
    out_format = in_format;
    auto output = checkSupport(gpu, in_format);

    if (output) {
        return output;
    }

    auto currFormats = format(gpu);
    for (const auto& currFormat : currFormats) {
        if (in_format.format == currFormat.format) {
            out_format.colorSpace = currFormat.colorSpace;
            POLYPINFO("Desired format is partially supported, setting colorspace to %d", out_format.colorSpace);
            return output;
        }
    }

    POLYPWARN("Desired format is not supported. Selecting available {format / colorspace combination}.");
    POLYPWARN("Image format - %d, color space - %d.",
               currFormats.begin()->format, currFormats.begin()->colorSpace);
    out_format.format = currFormats.begin()->format;
    out_format.colorSpace = currFormats.begin()->colorSpace;
    return output;
}

bool Surface::checkSupport(PhysicalGpu gpu, VkPresentModeKHR mode) const {
    auto currModes = presentModes(gpu);
    for (size_t i = 0; i < currModes.size(); ++i) {
        if (currModes[i] == mode) {
            return true;
        }
    }
    return false;
}

bool Surface::checkSupport(PhysicalGpu gpu, VkPresentModeKHR in_mode, VkPresentModeKHR out_mode) const {
    POLYPASSERT("");
    return false;
}

VkSurfaceCapabilitiesKHR Surface::capabilities(PhysicalGpu gpu) const {
    VkSurfaceCapabilitiesKHR output;
    CHECKRET(mInstance->vk().GetPhysicalDeviceSurfaceCapabilitiesKHR(*gpu, this->raw(), &output));
    return output;
}

std::vector<VkSurfaceFormatKHR> Surface::format(PhysicalGpu gpu) const {
    uint32_t count = 0;
    CHECKRET(mInstance->vk().GetPhysicalDeviceSurfaceFormatsKHR(*gpu, this->raw(), &count, nullptr));
    std::vector<VkSurfaceFormatKHR> output(count);
    CHECKRET(mInstance->vk().GetPhysicalDeviceSurfaceFormatsKHR(*gpu, this->raw(), &count, output.data()));
    return output;
}

std::vector<VkPresentModeKHR> Surface::presentModes(PhysicalGpu gpu) const {
    uint32_t count = 0;
    CHECKRET(mInstance->vk().GetPhysicalDeviceSurfacePresentModesKHR(*gpu, this->raw(), &count, nullptr));
    std::vector<VkPresentModeKHR> output(count);
    CHECKRET(mInstance->vk().GetPhysicalDeviceSurfacePresentModesKHR(*gpu, this->raw(), &count, output.data()));
    return output;
}

bool Surface::init() {
    VkWin32SurfaceCreateInfoKHR createInfo = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, mInfo.mWindowsInstance, mInfo.mWindowsHandle };
    CHECKRET(mInstance->vk().CreateWin32SurfaceKHR(mInstance->raw(), &createInfo, nullptr, &*mHandle));

    initVkDestroyer(mInstance->vk().DestroySurfaceKHR, mHandle, nullptr);

    return true;
}

} // engine
} // polyp
