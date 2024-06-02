#include "vk_context.h"
#include "polyp_config.h"
#include "polyp_logs.h"
#include "vk_utils.h"

using namespace polyp::vulkan::utils;

namespace polyp {
namespace vulkan {

namespace {

std::vector<bool> getSupportedQueueFamilies(const PhysicalDevice& gpu, QueueFlags flags, uint32_t queueCount)
{
    auto queFamilyProps = gpu.getQueueFamilyProperties();
    std::vector<bool> output(queFamilyProps.size(), false);

    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;

    for (uint32_t i = 0; i < queFamilyProps.size(); i++)
    {
        if ((queFamilyProps[i].queueFlags & flags) &&
            (queFamilyProps[i].queueCount >= queueCount))
        {
            output[i] = true;
        }
    }

    return output;
}

std::vector<bool> getSupportedQueueFamilies(const PhysicalDevice& gpu, const SurfaceKHR& surface)
{
    auto queFamilyProps = gpu.getQueueFamilyProperties();

    std::vector<bool> output(queFamilyProps.size());
    for (size_t i = 0; i < output.size(); i++)
    {
        output[i] = (gpu.getSurfaceSupportKHR(i, *surface) == VK_TRUE);
    }

    return output;
}

}

void RHIContext::init(const CreateInfo::Application& info)
{
    mCreateInfo.app = info;

    ApplicationInfo applicationInfo(info.name.c_str(), info.version, ENGINE_NAME, (ENGINE_MAJOR_VERSION << 16 + ENGINE_MINOR_VERSION), ENGINE_VK_VERSION);

    std::vector<const char*> extensions{
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME };

    std::vector<const char*> layers{};

#if defined(DEBUG)
    layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, layers, extensions);

    mInstance = Instance(mContext, instanceCreateInfo);
}

void RHIContext::init(const CreateInfo::GPU info)
{
    mCreateInfo.gpu = info;

    auto gpus = mInstance.enumeratePhysicalDevicesPLP();

    POLYPTODO("Check this code");
    std::sort(gpus.begin(), gpus.end(), [](const auto& lhv, const auto& rhv) {
        return lhv.getPerformanceRatioPLP() > rhv.getPerformanceRatioPLP();
        });

    if (gpus.empty())
        return;

    switch (info)
    {
    case CreateInfo::GPU::Powerful:
        mGPU = *gpus.begin();
        break;
    default:
        break;
    }
}

void RHIContext::init(const CreateInfo::Surface& info)
{
    mCreateInfo.win = info;

    Win32SurfaceCreateInfoKHR surfaceInfo({}, info.instance, info.handle, nullptr);
    mSurface = mInstance.createWin32SurfaceKHR(surfaceInfo);
}

void RHIContext::init(const CreateInfo::Device& info)
{
    mCreateInfo.device = info;

    DeviceCreateInfo                   deviceCreateInfo{};
    std::vector<DeviceQueueCreateInfo> queueCreateInfos(info.pQueueInfos.size());

    deviceCreateInfo.pQueueCreateInfos    = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();

    if (info.pQueueInfos.empty()) {
       POLYPERROR("Zero queue were requested");
       return;
    }

    std::vector<std::vector<float>> quePriorities(info.pQueueInfos.size());

    for (size_t i = 0; i < info.pQueueInfos.size(); i++)
    {
        const auto& queInfo = info.pQueueInfos[i];

        quePriorities[i] = std::vector<float>(queInfo.count, 1.);

        queueCreateInfos[i].pQueuePriorities = quePriorities[i].data();
        queueCreateInfos[i].queueCount       = quePriorities[i].size();
        queueCreateInfos[i].queueFamilyIndex = UINT32_MAX;

        auto queReqFlags = (queInfo.flags) ? queInfo.flags : QueueFlagBits::eGraphics;

        auto availableQueue    = getSupportedQueueFamilies(mGPU, queReqFlags, quePriorities[i].size());
        auto availableWSIQueue = getSupportedQueueFamilies(mGPU, mSurface);

        for (uint32_t i = 0; i < availableQueue.size(); i++) {
            if (availableQueue[i] && availableWSIQueue[i]) {
                queueCreateInfos[i].queueFamilyIndex = i;
                break;
            }
        }

        if (queueCreateInfos[i].queueFamilyIndex == UINT32_MAX) {
            POLYPERROR("Failed to find the reqired queue family");
            return;
        }

        mQueueFamilies[queInfo.flags] = queueCreateInfos[i].queueFamilyIndex;
    }

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

    PhysicalDeviceFeatures deviceFeatures = info.features;
    deviceCreateInfo.pEnabledFeatures     = &deviceFeatures;

    {
        auto availableFeatures = static_cast<VkPhysicalDeviceFeatures>(mGPU.getFeatures());
        auto requestedFeatures = static_cast<VkPhysicalDeviceFeatures>(info.features);

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

void RHIContext::init(const CreateInfo::SwapChain& info)
{
    mCreateInfo.swapchain = info;

    PresentModeKHR reqPresentMode = PresentModeKHR::eMailbox;

    SwapchainCreateInfoKHR createInfo{};
    createInfo.surface          = *mSurface;
    createInfo.minImageCount    = info.count;
    createInfo.imageFormat      = Format::eR8G8B8A8Unorm;
    createInfo.imageColorSpace  = ColorSpaceKHR::eSrgbNonlinear;
    createInfo.presentMode      = reqPresentMode;
    createInfo.imageUsage       = ImageUsageFlagBits::eColorAttachment;
    createInfo.imageExtent      = mGPU.getSurfaceCapabilitiesKHR(*mSurface).currentExtent;
    createInfo.imageArrayLayers = 1; // single layer, no stereoscopic-3D
    createInfo.imageSharingMode = SharingMode::eExclusive; // image is owned by one queue family at a time
    createInfo.preTransform     = SurfaceTransformFlagBitsKHR::eIdentity;
    createInfo.compositeAlpha   = CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.clipped          = true; // enable clipping
    createInfo.oldSwapchain     = *mSwapchain;

    if (!mGPU.supportPLP(mSurface, createInfo.presentMode)) {
        auto modes = mGPU.getSurfacePresentModesKHR(*mSurface);
        if (modes.empty()) {
            POLYPFATAL("Internal error");
            return;
        }

        createInfo.presentMode = modes[0];
        POLYPWARN("Requested presentation %s mode is not found. Will be used %s",
                   to_string(reqPresentMode).c_str(),
                   to_string(createInfo.presentMode).c_str());
    }

    mSwapchain = mDevice.createSwapchainKHR(createInfo);
}

uint32_t RHIContext::queueFamily(QueueFlags flags) const
{
    auto familyIt = mQueueFamilies.find(flags);
    if (familyIt == mQueueFamilies.end()) {
        POLYPERROR("Queue family index is absent for provided QueueFlags %s", to_string(flags).c_str());
        return UINT32_MAX;
    }

    return familyIt->second;
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

    init(info.swapchain);
    if (*mSwapchain == VK_NULL_HANDLE) {
        POLYPERROR("Failed to initialize Vulkan swapchain");
        return;
    }
}

}
}
