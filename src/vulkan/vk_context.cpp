#include "vk_context.h"
#include "vk_utils.h"

using namespace polyp::vulkan::utils;

namespace polyp {
namespace vulkan {

namespace {

std::vector<bool> getSupportedQueueFamilies(const std::vector<vk::QueueFamilyProperties>& props, QueueFlags flags, uint32_t queueCount)
{
    std::vector<bool> output(props.size(), false);

    uint32_t graphicsQueueFamilyIndex = UINT32_MAX;

    for (uint32_t i = 0; i < props.size(); i++)
    {
        if ((props[i].queueFlags & flags) &&
            (props[i].queueCount >= queueCount))
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

    ApplicationInfo applicationInfo(info.name.c_str(), info.version, ENGINE_NAME, ENGINE_VERSION, ENGINE_VK_VERSION);

    std::vector<const char*> extensions
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

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

    if (info.handle == NULL || info.instance == NULL)
    {
        POLYPERROR("Empty surface info provided. WSI will no be created.");
        return;
    }

    Win32SurfaceCreateInfoKHR surfaceInfo({}, info.instance, info.handle, nullptr);
    mSurface = mInstance.createWin32SurfaceKHR(surfaceInfo);
}

void RHIContext::init(const CreateInfo::Device& info)
{
    mCreateInfo.device = info;

    DeviceCreateInfo                   deviceCreateInfo{};
    std::vector<DeviceQueueCreateInfo> queueCreateInfos(info.queues.size());

    deviceCreateInfo.pQueueCreateInfos    = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();

    if (info.queues.empty())
    {
       POLYPERROR("Zero queue were requested");
       return;
    }

    auto queProps = mGPU.getQueueFamilyProperties();

    auto availableWSIQueue = getSupportedQueueFamilies(mGPU, mSurface);

    std::vector<std::vector<float>> quePriorities(info.queues.size());

    for (size_t i = 0; i < info.queues.size(); ++i)
    {
        const auto& queInfo = info.queues[i];

        POLYPASSERTNOTEQUAL(queInfo.flags, static_cast<QueueFlagBits>(0));

        quePriorities[i] = std::vector<float>(queInfo.count, 1.);

        queueCreateInfos[i].pQueuePriorities = quePriorities[i].data();
        queueCreateInfos[i].queueCount       = quePriorities[i].size();
        queueCreateInfos[i].queueFamilyIndex = UINT32_MAX;

        auto availableQueue = getSupportedQueueFamilies(queProps, queInfo.flags, queInfo.count);

        for (uint32_t j = 0; j < availableQueue.size(); ++j)
        {
            if (availableQueue[j] && queProps[j].queueCount >= queInfo.count)
            {
                if (queInfo.isWSIRequred && availableWSIQueue[i])
                {
                    queueCreateInfos[i].queueFamilyIndex = j;
                    queProps[j].queueCount -= queInfo.count;
                    break;
                }
                else if (!queInfo.isWSIRequred)
                {
                    queueCreateInfos[i].queueFamilyIndex = j;
                    queProps[j].queueCount -= queInfo.count;
                    break;
                }
            }
        }

        if (queueCreateInfos[i].queueFamilyIndex == UINT32_MAX)
        {
            POLYPERROR("Failed to find the reqired queue family");
            return;
        }

        if (mQueueFamilies.contains(queInfo.flags))
        {
            POLYPERROR("Internal error: only one entity with unique queue flags is available.");
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
        for (size_t i = 0, j = 0; i < extansions.size() && j < available.size(); j++)
        {
            if (strcmp(extansions[i], available[j].extensionName.data()) == 0)
            {
                foundCount++;
                i++;
            }
        }

        if (foundCount != extansions.size())
        {
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

    auto device = mGPU.createDevice(deviceCreateInfo).release();
    mDevice = Device(static_cast<vk::raii::PhysicalDevice&>(mGPU), device);
}

void RHIContext::init(const CreateInfo::SwapChain& info)
{
    mCreateInfo.swapchain = info;

    PresentModeKHR reqPresentMode = PresentModeKHR::eMailbox;

    auto surfaceFormat = mGPU.getColorFormatPLP(mSurface);
    auto capabilities  = mGPU.getSurfaceCapabilitiesKHR(*mSurface);

    SwapchainCreateInfoKHR createInfo{};
    createInfo.surface          = *mSurface;
    createInfo.minImageCount    = info.count;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.presentMode      = reqPresentMode;
    createInfo.imageUsage       = ImageUsageFlagBits::eColorAttachment;
    createInfo.imageExtent      = capabilities.currentExtent;
    createInfo.imageArrayLayers = 1; // single layer, no stereoscopic-3D
    createInfo.imageSharingMode = SharingMode::eExclusive; // image is owned by one queue family at a time
    createInfo.preTransform     = SurfaceTransformFlagBitsKHR::eIdentity;
    createInfo.compositeAlpha   = CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.clipped          = true; // enable clipping
    createInfo.oldSwapchain     = *mSwapchain;

    if (!mGPU.supportPLP(mSurface, createInfo.presentMode))
    {
        auto modes = mGPU.getSurfacePresentModesKHR(*mSurface);
        if (modes.empty()) 
        {
            detail::throwResultException(Result::eErrorOutOfHostMemory, __FUNCTION__);
            return;
        }

        createInfo.presentMode = modes[0];
        POLYPWARN("Requested presentation %s mode is not found. Will be used %s",
                   to_string(reqPresentMode).c_str(),
                   to_string(createInfo.presentMode).c_str());
    }
    mSwapchain = mDevice.createSwapchainPLP(createInfo);
}

uint32_t RHIContext::queueFamily(QueueFlags flags) const
{
    auto familyIt = mQueueFamilies.find(flags);
    if (familyIt == mQueueFamilies.end())
    {
        POLYPERROR("Queue family index is absent for provided QueueFlags %s", to_string(flags).c_str());
        return UINT32_MAX;
    }

    return familyIt->second;
}

void RHIContext::init(const CreateInfo& info)
{
    clear();

    init(info.app);
    if (*mInstance == VK_NULL_HANDLE)
    {
        POLYPERROR("Failed to initialize Vulkan instance");
        return;
    }

    init(info.gpu);
    if (*mGPU == VK_NULL_HANDLE)
    {
        POLYPERROR("Required GPU not found");
        return;
    }

    init(info.win);
    if (*mSurface == VK_NULL_HANDLE)
    {
        POLYPERROR("Failed to initialize Vulkan surface (WSI)");
        return;
    }

    init(info.device);
    if (*mDevice == VK_NULL_HANDLE)
    {
        POLYPERROR("Failed to initialize Vulkan logical device");
        return;
    }

    init(info.swapchain);
    if (*mSwapchain == VK_NULL_HANDLE)
    {
        POLYPERROR("Failed to initialize Vulkan swapchain");
        return;
    }
}

}
}
