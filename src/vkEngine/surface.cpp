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

std::vector<bool> Surface::checkSupport(GpuInfo gpu) {
    std::vector<bool> output(gpu.queueFamilyCount());
    VkBool32 flag = VK_FALSE;
    for (size_t i = 0; i < output.size(); i++) {
        CHECKRET(mInstance->getDispatchTable().GetPhysicalDeviceSurfaceSupportKHR(*gpu, i, **this, &flag));
        output[i] = flag == VK_TRUE;
    }
    return output;
}

bool Surface::init() {
    VkWin32SurfaceCreateInfoKHR createInfo = {
        VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, mInfo.mWindow.inst, mInfo.mWindow.hwnd };
    CHECKRET(mInstance->getDispatchTable().CreateWin32SurfaceKHR(**mInstance, &createInfo, nullptr, &*mHandle));

    initVkDestroyer(mInstance->getDispatchTable().DestroySurfaceKHR, mHandle, nullptr);

    return true;
}

} // engine
} // polyp
