#pragma once

#include "common.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace polyp {
namespace vulkan {

using namespace vk;

using Context             = vk::raii::Context;
using SurfaceKHR          = vk::raii::SurfaceKHR;
using SwapchainKHR        = vk::raii::SwapchainKHR;
using ImageView           = vk::raii::ImageView;
using RenderPass          = vk::raii::RenderPass;
using Queue               = vk::raii::Queue;
using CommandPool         = vk::raii::CommandPool;
using CommandBuffer       = vk::raii::CommandBuffer;
using Semaphore           = vk::raii::Semaphore;
using Fence               = vk::raii::Fence;
using Framebuffer         = vk::raii::Framebuffer;
using Pipeline            = vk::raii::Pipeline;
using DeviceMemory        = vk::raii::DeviceMemory;
using DescriptorSetLayout = vk::raii::DescriptorSetLayout;
using PipelineLayout      = vk::raii::PipelineLayout;
using DescriptorPool      = vk::raii::DescriptorPool;
using DescriptorSet       = vk::raii::DescriptorSet;
using ShaderModule        = vk::raii::ShaderModule;

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
        init(physicalDevice);
    }

    Device(vk::raii::PhysicalDevice const&     physicalDevice,
           VkDevice                            device,
           Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Device(physicalDevice, device, allocator)
    {
        init(physicalDevice);
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
        vk::raii::Device::operator=(static_cast<vk::raii::Device&&>(rhv));

        std::swap(mAllocatorVMA, rhv.mAllocatorVMA);

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

    void init(vk::raii::PhysicalDevice const& gpu);
};

class Image : protected vk::raii::Image
{
public:
    friend class Device;

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

    vk::Image const& operator*() const noexcept
    {
        return vk::raii::Image::operator*();
    }

private:
    Image(Device const& device,
          VkImage                             image,
          VmaAllocation                       vmaAllocation,
          VmaAllocationInfo const& vmaAllocationInfo,
          Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Image(device, image, allocator)
    {
        mAllocationVMA = vmaAllocation;
        mAllocationVMAInfo = vmaAllocationInfo;
    }

    VmaAllocation         mAllocationVMA = VK_NULL_HANDLE;
    VmaAllocationInfo mAllocationVMAInfo = {};
};

class Buffer : protected vk::raii::Buffer
{
public:
    friend class Device;

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

    vk::Buffer const& operator*() const noexcept
    {
        return vk::raii::Buffer::operator*();
    }

    void fill(void* data, VkDeviceSize size, VkDeviceSize offset = 0);

    template<typename Container>
    void fill(const Container& data, VkDeviceSize offset = 0)
    {
        auto size = sizeof(Container::value_type) * data.size();
        fill((void*)data.data(), size, offset);
    }

private:
    Buffer(Device const& device,
           VkBuffer                            buffer,
           VmaAllocation                       vmaAllocation,
           VmaAllocationInfo const& vmaAllocationInfo,
           Optional<const AllocationCallbacks> allocator = nullptr) :
        vk::raii::Buffer(device, buffer, allocator)
    {
        mAllocationVMA = vmaAllocation;
        mAllocationVMAInfo = vmaAllocationInfo;
    }

    VmaAllocation         mAllocationVMA = VK_NULL_HANDLE;
    VmaAllocationInfo mAllocationVMAInfo = {};
};

}
}