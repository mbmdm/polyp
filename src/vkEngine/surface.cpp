#include "surface.h"

namespace polyp {
namespace engine {

Surface::Surface(Instance::Ptr instance, const SurfaceCreateInfo& info) : 
                 mInstance{ instance }, mInfo{ info }, 
                 mHandle{ { **instance, VK_NULL_HANDLE }, nullptr } {
}

const VkSurfaceKHR& Surface::operator*() const {
    return *mHandle;
}

std::vector<bool> Surface::checkSupport(GpuInfo gpuInfo) const {
    std::vector<bool> output(gpuInfo.queueFamilyCount());
    VkBool32 flag = VK_FALSE;
    for (size_t i = 0; i < output.size(); i++) {
        CHECKRET(mInstance->dispatchTable().GetPhysicalDeviceSurfaceSupportKHR(*gpuInfo, i, **this, &flag));
        output[i] = flag == VK_TRUE;
    }
    return output;
}

bool Surface::checkSupport(GpuInfo gpuInfo, VkSurfaceFormatKHR format) const {
    auto currFormats = formats(gpuInfo);
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

bool Surface::checkSupport(GpuInfo gpuInfo, VkSurfaceFormatKHR in_format,
                           VkSurfaceFormatKHR& out_format) const {
    out_format = in_format;
    auto output = checkSupport(gpuInfo, in_format);

    if (output) {
        return output;
    }

    auto currFormats = formats(gpuInfo);
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

VkSurfaceCapabilitiesKHR Surface::capabilities(GpuInfo gpuInfo) const {
    VkSurfaceCapabilitiesKHR output;
    CHECKRET(mInstance->dispatchTable().GetPhysicalDeviceSurfaceCapabilitiesKHR(*gpuInfo, **this, &output));
    return output;
}

std::vector<VkSurfaceFormatKHR> Surface::formats(GpuInfo gpuInfo) const {
    uint32_t count = 0;
    CHECKRET(mInstance->dispatchTable().GetPhysicalDeviceSurfaceFormatsKHR(*gpuInfo, **this, &count, nullptr));
    std::vector<VkSurfaceFormatKHR> output(count);
    CHECKRET(mInstance->dispatchTable().GetPhysicalDeviceSurfaceFormatsKHR(*gpuInfo, **this, &count, output.data()));
    return output;
}

bool Surface::init() {
    VkWin32SurfaceCreateInfoKHR createInfo = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, mInfo.mWindow.inst, mInfo.mWindow.hwnd };
    CHECKRET(mInstance->dispatchTable().CreateWin32SurfaceKHR(**mInstance, &createInfo, nullptr, &*mHandle));

    initVkDestroyer(mInstance->dispatchTable().DestroySurfaceKHR, mHandle, nullptr);

    return true;
}

} // engine
} // polyp
