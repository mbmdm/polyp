#pragma once

#include "common.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

namespace polyp {
namespace vulkan {

using namespace vk;

using Context       = vk::raii::Context;
using SurfaceKHR    = vk::raii::SurfaceKHR;
using SwapchainKHR  = vk::raii::SwapchainKHR;
using ImageView     = vk::raii::ImageView;
using RenderPass    = vk::raii::RenderPass;
using Queue         = vk::raii::Queue;
using CommandPool   = vk::raii::CommandPool;
using CommandBuffer = vk::raii::CommandBuffer;
using Semaphore     = vk::raii::Semaphore;
using Fence         = vk::raii::Fence;
using Framebuffer   = vk::raii::Framebuffer;

class PhysicalDevice;
class Instance;
class Device;
class Image;
class Buffer;

class PhysicalDevice : public vk::raii::PhysicalDevice
{
public:
    PhysicalDevice(vk::raii::Instance const& instance, 
                   VkPhysicalDevice          physicalDevice) :
        vk::raii::PhysicalDevice(instance, physicalDevice)
    { }

    PhysicalDevice(vk::raii::PhysicalDevice physicalDevice) :
        vk::raii::PhysicalDevice(std::move(physicalDevice))
    { }

    PhysicalDevice(std::nullptr_t ptr) :
        vk::raii::PhysicalDevice(ptr)
    { }

    VkDeviceSize getDeviceMemoryPLP() const;

    uint64_t getPerformanceRatioPLP() const;

    bool supportPLP(const SurfaceKHR& surface, PresentModeKHR mode) const;

    bool isDiscretePLP() const;

    Format getDepthFormatPLP() const;

    std::string toStringPLP() const;
};

class Instance : public vk::raii::Instance
{
public:
    Instance(vk::raii::Context const&           context, 
            InstanceCreateInfo const&           createInfo, 
            Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Instance(context, createInfo, allocator)
    { }

    Instance(vk::raii::Context const&            context,
             VkInstance                          instance,
             Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Instance(context, instance, allocator)
    { }

    Instance(std::nullptr_t ptr) :
        vk::raii::Instance(ptr)
    { }

    std::vector<PhysicalDevice> enumeratePhysicalDevicesPLP() const;
};

class Device : public vk::raii::Device
{
public:
    Device(vk::raii::PhysicalDevice const&     physicalDevice,
           DeviceCreateInfo const&             createInfo,
           Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Device(physicalDevice, createInfo, allocator)
    {
        init();
    }

    Device(vk::raii::PhysicalDevice const&     physicalDevice,
           VkDevice                            device,
           Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Device(physicalDevice, device, allocator)
    {
        init();
    }

    Device(std::nullptr_t ptr) :
        vk::raii::Device(ptr)
    { }

    Device()                         = delete;
    Device(const Device&)            = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&& rhv) noexcept :
        vk::raii::Device(static_cast<vk::raii::Device&&>(rhv))
    {
        std::swap(mAllocatorVMA, rhv.mAllocatorVMA);
    }

    Device& operator=(Device&& rhv) noexcept
    {
        *this = static_cast<vk::raii::Device&&>(rhv);
        std::swap(mAllocatorVMA, rhv.mAllocatorVMA);
        return *this;
    }

    vk::raii::Device& operator=(vk::raii::Device&& rhv) noexcept
    {
        static_cast<vk::raii::Device&>(*this) = std::move(rhv);
        init();
        return *this;
    }

    VmaAllocator vmaAlocator() const { return mAllocatorVMA; }

    ~Device()
    {
        vmaDestroyAllocator(mAllocatorVMA);
    }

    Image createImagePLP(const ImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo) const;

    Buffer createBufferPLP(const BufferCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo) const;

private:
    VmaAllocator mAllocatorVMA = { VK_NULL_HANDLE };

    void init();
};

class Image : public vk::raii::Image
{
public:
    Image(vk::raii::Device const&             device,
          ImageCreateInfo const&              createInfo,
          Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Image(device, createInfo, allocator)
    { }

    Image(Device const& device, VkImage image, Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Image(device, image, allocator)
    { }

    Image(Device const&                       device, 
          VkImage                             image, 
          VmaAllocation                       vmaAllocation, 
          VmaAllocationInfo const&            vmaAllocationInfo, 
          Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Image(device, image, allocator)
    {
        mAllocationVMA     = vmaAllocation;
        mAllocationVMAInfo = vmaAllocationInfo;
    }

    Image(std::nullptr_t ptr) :
        vk::raii::Image(ptr)
    { }

    Image() = delete;
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    Image(Image&& rhv) noexcept :
        vk::raii::Image(static_cast<vk::raii::Image&&>(rhv))
    {
        std::swap(mAllocationVMA,     rhv.mAllocationVMA);
        std::swap(mAllocationVMAInfo, rhv.mAllocationVMAInfo);
    }

    Image& operator=(Image&& rhv) noexcept
    {
        vk::raii::Image::operator=(static_cast<vk::raii::Image&&>(rhv));

        std::swap(mAllocationVMA,     rhv.mAllocationVMA);
        std::swap(mAllocationVMAInfo, rhv.mAllocationVMAInfo);

        return *this;
    }

    ~Image();

private:
    VmaAllocation         mAllocationVMA = VK_NULL_HANDLE;
    VmaAllocationInfo mAllocationVMAInfo = {};
};

class Buffer : public vk::raii::Buffer
{
public:
    Buffer(vk::raii::Device const&             device,
           vk::BufferCreateInfo const&         createInfo,
           Optional<const AllocationCallbacks> allocator = nullptr):
        vk::raii::Buffer(device, createInfo, allocator)
    { }

    Buffer(Device const& device, VkBuffer buffer, Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Buffer(device, buffer, allocator)
    { }

    Buffer(Device const&                       device,
           VkBuffer                            buffer,
           VmaAllocation                       vmaAllocation,
           VmaAllocationInfo const&            vmaAllocationInfo,
           Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Buffer(device, buffer, allocator)
    {
        mAllocationVMA     = vmaAllocation;
        mAllocationVMAInfo = vmaAllocationInfo;
    }

    Buffer(std::nullptr_t ptr) :
        vk::raii::Buffer(ptr)
    { }

    Buffer() = delete;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& rhv) noexcept :
        vk::raii::Buffer(static_cast<vk::raii::Buffer&&>(rhv))
    {
        std::swap(mAllocationVMA,     rhv.mAllocationVMA);
        std::swap(mAllocationVMAInfo, rhv.mAllocationVMAInfo);
    }

    Buffer& operator=(Buffer&& rhv) noexcept
    {
        vk::raii::Buffer::operator=(static_cast<vk::raii::Buffer&&>(rhv));

        std::swap(mAllocationVMA,     rhv.mAllocationVMA);
        std::swap(mAllocationVMAInfo, rhv.mAllocationVMAInfo);

        return *this;
    }

    ~Buffer();

private:
    VmaAllocation         mAllocationVMA = VK_NULL_HANDLE;
    VmaAllocationInfo mAllocationVMAInfo = {};
};

//
//    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
//    bufferInfo.size = 65536;
//    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
//
//    VmaAllocationCreateInfo vmaAllocInfo = {};
//    vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
//
//    VkBuffer buffer;
//    VmaAllocation allocation;
//    vkres = vk::Result(vmaCreateBuffer(mVmaAllocator, &bufferInfo, &vmaAllocInfo, &buffer, &allocation, nullptr));
//    if (vkres != vk::Result::eSuccess) {
//        POLYPFATAL("Failed to create VkBuffer through VMA.");
//    }


}
}