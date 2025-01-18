#define VMA_IMPLEMENTATION

#include "vk_common.h"
#include "vk_context.h"

namespace polyp {
namespace vulkan {

std::vector<PhysicalDevice> Instance::enumeratePhysicalDevicesPLP() const
{
    auto gpus = enumeratePhysicalDevices();

    std::vector<PhysicalDevice> output;

    std::transform(gpus.begin(), gpus.end(), std::back_inserter(output), [](auto& gpu) { return gpu; });

    return output;
}

VkDeviceSize PhysicalDevice::getDeviceMemoryPLP() const
{
    VkDeviceSize output = 0;

    vk::PhysicalDeviceMemoryProperties memProperties = getMemoryProperties();

    std::vector<size_t> targetHeapsIdx;
    for (size_t heapTypeIdx = 0; heapTypeIdx < memProperties.memoryTypeCount; ++heapTypeIdx) {
        if (memProperties.memoryTypes[heapTypeIdx].propertyFlags & MemoryPropertyFlagBits::eDeviceLocal) {
            targetHeapsIdx.push_back(memProperties.memoryTypes[heapTypeIdx].heapIndex);
        }
    }

    if (targetHeapsIdx.empty()) {
        return output;
    }

    //remove the same indexes if exist
    std::sort(targetHeapsIdx.begin(), targetHeapsIdx.end(), std::less<size_t>());
    auto currItr = targetHeapsIdx.begin() + 1;
    while (currItr != targetHeapsIdx.end()) {
        if (*currItr == *(currItr - 1)) {
            currItr = targetHeapsIdx.erase(currItr);
        }
        else {
            currItr++;
        }
    }

    for (size_t i = 0; i < targetHeapsIdx.size(); i++) {
        output += memProperties.memoryHeaps[targetHeapsIdx[i]].size;
    }

    return output;
}

bool PhysicalDevice::isDiscretePLP() const
{
    PhysicalDeviceProperties props = getProperties();
    return props.deviceType == PhysicalDeviceType::eDiscreteGpu;
}

Format PhysicalDevice::getDepthFormatPLP() const
{
    std::vector<vk::Format> dsDesiredFormats = {
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD32Sfloat,
        vk::Format::eD24UnormS8Uint,
        vk::Format::eD16UnormS8Uint,
        vk::Format::eD16Unorm
    };

    auto depthFormat = vk::Format::eUndefined;
    for (const auto& format : dsDesiredFormats) {
        auto props = getFormatProperties(format);
        if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            depthFormat = format;
            break;
        }
    }

    return depthFormat;
}

std::string PhysicalDevice::toStringPLP() const
{
    auto props = getProperties();

    std::stringstream ss;
    ss << props.deviceName;

    auto memory = getDeviceMemoryPLP();
    ss << ", memory " << memory / 1024 / 1024 << " Mb";

    return ss.str();
}

uint64_t PhysicalDevice::getPerformanceRatioPLP() const
{
    uint64_t ratio = getDeviceMemoryPLP() >> 20;
    if (!isDiscretePLP())
        ratio |= 1ULL << 32;
    
    return ratio;
}

bool PhysicalDevice::supportPLP(const SurfaceKHR& surface, PresentModeKHR mode) const
{
    auto capabilities = getSurfaceCapabilitiesKHR(*surface);
    if (capabilities.currentExtent.width == UINT32_MAX ||
        capabilities.currentExtent.height == UINT32_MAX) {
        return false;
    }
    else if (capabilities.currentExtent.width == 0 ||
        capabilities.currentExtent.height == 0) {
        return false;
    }

    auto presentModes = getSurfacePresentModesKHR(*surface);
    for (size_t i = 0; i < presentModes.size(); ++i) {
        if (presentModes[i] == mode) {
            return true;
            break;
        }
    }

    return false;
}

Image::~Image()
{
    if (mAllocationVMA != VK_NULL_HANDLE)
    {
        auto resource  = release();
        auto allocator = RHIContext::get().device().vmaAlocator();

        vmaDestroyImage(allocator, static_cast<VkImage>(resource), mAllocationVMA);
    }
}

Image Device::createImagePLP(const ImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo) const
{
    vk::Image         resource;
    VmaAllocation     allocation;
    VmaAllocationInfo allocationInfo;
    
    auto res = vmaCreateImage(mAllocatorVMA, reinterpret_cast<const VkImageCreateInfo*>(&createInfo), 
                              &allocationCreateInfo, reinterpret_cast<VkImage*>(&resource), &allocation, &allocationInfo);
    if (res != VK_SUCCESS) {
        detail::throwResultException(static_cast<vk::Result>(res), __FUNCTION__);
    }

    return Image(*this, *reinterpret_cast<VkImage*>(&resource), allocation, allocationInfo);
}

Buffer Device::createBufferPLP(const BufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo) const
{
    vk::Buffer        resource;
    VmaAllocation     allocation;
    VmaAllocationInfo allocationInfo;

    auto res = vmaCreateBuffer(mAllocatorVMA, reinterpret_cast<const VkBufferCreateInfo*>(&createInfo),
        &allocationCreateInfo, reinterpret_cast<VkBuffer*>(&resource), &allocation, &allocationInfo);
    if (res != VK_SUCCESS) {
        detail::throwResultException(static_cast<vk::Result>(res), __FUNCTION__);
    }

    return Buffer(*this, *reinterpret_cast<VkBuffer*>(&resource), allocation, allocationInfo);
}

void Device::init(vk::raii::PhysicalDevice const& gpu)
{
    if (static_cast<VkDevice>(**this) == VK_NULL_HANDLE)
        return;

    auto* devDispatcher  = getDispatcher();
    auto* instDispatcher = gpu.getDispatcher();

    VmaVulkanFunctions vulkanFunctions = {};

    vulkanFunctions.vkGetInstanceProcAddr = instDispatcher->vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr   = devDispatcher->vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.flags            = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    allocatorCreateInfo.vulkanApiVersion = ENGINE_VK_VERSION;
    allocatorCreateInfo.physicalDevice   = static_cast<VkPhysicalDevice>(*RHIContext::get().gpu());
    allocatorCreateInfo.device           = static_cast<VkDevice>(**this);
    allocatorCreateInfo.instance         = static_cast<VkInstance>(*RHIContext::get().instance());;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    auto vkres = vk::Result(vmaCreateAllocator(&allocatorCreateInfo, &mAllocatorVMA));
    if (vkres != vk::Result::eSuccess) {
        POLYPERROR("Failed to create VMA allocator.");
        mAllocatorVMA = VK_NULL_HANDLE;
    }
}

Buffer::~Buffer()
{
    if (mAllocationVMA != VK_NULL_HANDLE)
    {
        auto resource  = release();
        auto allocator = RHIContext::get().device().vmaAlocator();

        vmaDestroyBuffer(allocator, static_cast<VkBuffer>(resource), mAllocationVMA);
    }
}

void Buffer::fill(void* data, VkDeviceSize size, VkDeviceSize offset)
{
    auto& device = RHIContext::get().device();
    auto allocator = device.vmaAlocator();

    auto res = vmaCopyMemoryToAllocation(allocator, data, mAllocationVMA, offset, size);

    if (res != VK_SUCCESS) {
        detail::throwResultException(static_cast<vk::Result>(res), __FUNCTION__);
    }
}

}
}