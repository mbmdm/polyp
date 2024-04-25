#include "vk_context.h"
#include "polyp_config.h"
#include "polyp_logs.h"
#include "vk_utils.h"

using namespace polyp::vulkan::utils;

namespace polyp {
namespace vulkan {

void RHIContext::init(const CreateInfo::Application& info)
{
    vk::ApplicationInfo applicationInfo(info.name.c_str(), info.version, ENGINE_NAME, (ENGINE_MAJOR_VERSION << 16 + ENGINE_MINOR_VERSION), ENGINE_VK_VERSION);

    std::vector<const char*> extensions{
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

    std::vector<const char*> layers{};

#if defined(DEBUG)
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, layers, extensions);

    mInstance = vk::raii::Instance(mContext, instanceCreateInfo);
}

void RHIContext::init(const CreateInfo::GPU info)
{
    auto gpus = mInstance.enumeratePhysicalDevices();

    switch (info)
    {
    case CreateInfo::GPU::Powerful:
        mGPU = getPowerfullGPU(gpus);
        break;
    default:
        break;
    }
}

void RHIContext::init(const CreateInfo::Surface& info)
{
    vk::Win32SurfaceCreateInfoKHR surfaceInfo({}, info.instance, info.handle, nullptr);
    mSurface = mInstance.createWin32SurfaceKHR(surfaceInfo);
}

void RHIContext::init(const CreateInfo::Device& deviceInfo)
{
    vk::DeviceCreateInfo      deviceCreateInfo{};
    vk::DeviceQueueCreateInfo queueCreateInfo{};

    const auto& queInfo = deviceInfo.queue;

    if (queInfo.count == 0) {
       POLYPERROR("Zero queue were requested");
       return;
    }

    std::vector<float> quePriorities(queInfo.count, 1.);

    queueCreateInfo.pQueuePriorities = quePriorities.data();
    queueCreateInfo.queueCount       = quePriorities.size();
    queueCreateInfo.queueFamilyIndex = UINT32_MAX;

    {
        auto queReqFlags = (queInfo.flags) ? queInfo.flags : vk::QueueFlagBits::eGraphics;

        auto availableQueue    = getSupportedQueueFamilies(mGPU, queReqFlags, quePriorities.size());
        auto availableWSIQueue = getSupportedQueueFamilies(mGPU, mSurface);

        for (uint32_t i = 0; i < availableQueue.size(); i++) {
            if (availableQueue[i] && availableWSIQueue[i]) {
                queueCreateInfo.queueFamilyIndex = i;
                break;
            }
        }
    }

    if (queueCreateInfo.queueFamilyIndex == UINT32_MAX) {
        POLYPERROR("Failed to find the reqired queue family");
        return;
    }

    mQueueFamilyIndex = queueCreateInfo.queueFamilyIndex;

    std::vector<const char*> extansions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    deviceCreateInfo.ppEnabledExtensionNames = extansions.data();
    deviceCreateInfo.enabledExtensionCount   = extansions.size();

    {
        auto available = mGPU.enumerateDeviceExtensionProperties();
        sort(available.begin(), available.end(), [](const auto& lhv, const auto& rhv) {
            return strcmp(lhv.extensionName.data(), rhv.extensionName.data()) > 0;
            });

        sort(extansions.begin(), extansions.end(), strcmp);

        uint32_t foundCount = 0;
        for (size_t i = 0, j = 0; i < extansions.size() && j < available.size(); j++) {
            if (strcmp(extansions[i], available[j].extensionName.data()) == 0) {
                foundCount++;
                i++;
            }
        }

        if (foundCount != extansions.size()) {
            POLYPERROR("Physical GPU doesn't support all the requrested extansions. All the avalalble Vulkan extansions will be turned on.");
            extansions.clear();
            std::transform(available.begin(), available.end(), std::back_inserter(extansions), [](const auto& ext) {
                return ext.extensionName.data();
                });
        }
    }

    vk::PhysicalDeviceFeatures deviceFeatures = deviceInfo.features;
    deviceCreateInfo.pEnabledFeatures         = &deviceFeatures;

    {
        auto availableFeatures = static_cast<VkPhysicalDeviceFeatures>(mGPU.getFeatures());
        auto requestedFeatures = static_cast<VkPhysicalDeviceFeatures>(deviceInfo.features);

        VkBool32* available = (VkBool32*)&availableFeatures;
        VkBool32* requested = (VkBool32*)&requestedFeatures;

        auto count = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
        for (size_t i = 0; i < count; i++)
        {
            if (requested[i] && !available[i])
            {
                POLYPERROR("Physical GPU doesn't support all the requrested features. All the avalalble Vulkan feature will be turned on.");
                deviceFeatures = mGPU.getFeatures();
                break;
            }
        }
    }

    mDevice = mGPU.createDevice(deviceCreateInfo);
}

void RHIContext::init(const CreateInfo& info)
{
    clear();

    init(info.app);
    if (*mInstance == VK_NULL_HANDLE) {
        POLYPERROR("Failed to initialize Vulkan instance");
        return;
    }

    init(info.gpu);
    if (*mGPU == VK_NULL_HANDLE) {
        POLYPERROR("Required GPU not found");
        return;
    }

    init(info.win);
    if (*mSurface == VK_NULL_HANDLE) {
        POLYPERROR("Failed to initialize Vulkan surface (WSI)");
        return;
    }

    init(info.device);
    if (*mDevice == VK_NULL_HANDLE) {
        POLYPERROR("Failed to initialize Vulkan logical device");
        return;
    }
}

}
}
