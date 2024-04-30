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
        std::swap(mAllocationVMA, rhv.mAllocationVMA);
        std::swap(mAllocationVMAInfo, rhv.mAllocationVMAInfo);
    }

    Image& operator=(Image&& rhv) noexcept
    {
        vk::raii::Image::operator=(static_cast<vk::raii::Image&&>(rhv));

        std::swap(mAllocationVMA, rhv.mAllocationVMA);
        std::swap(mAllocationVMAInfo, rhv.mAllocationVMAInfo);

        return *this;
    }

    ~Image();

private:
    VmaAllocation         mAllocationVMA = VK_NULL_HANDLE;
    VmaAllocationInfo mAllocationVMAInfo = {};
};

//class Buffer : public vk::raii::Buffer
//{
//    //public:
//    //  using CType   = VkBuffer;
//    //  using CppType = vk::Buffer;
//
//      //static VULKAN_HPP_CONST_OR_CONSTEXPR VULKAN_HPP_NAMESPACE::ObjectType objectType = VULKAN_HPP_NAMESPACE::ObjectType::eBuffer;
//      //static VULKAN_HPP_CONST_OR_CONSTEXPR VULKAN_HPP_NAMESPACE::DebugReportObjectTypeEXT debugReportObjectType = VULKAN_HPP_NAMESPACE::DebugReportObjectTypeEXT::eBuffer;
//
//public:
//    Buffer(std::nullptr_t) {}
//
//    ~Buffer()
//    {
//        destroy();
//    }
//
//    Buffer() = delete;
//    Buffer(Buffer const&) = delete;
//
//    //need to implement
//    //Buffer( Buffer && rhs ) noexcept
//    //  : m_device( VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::exchange( rhs.m_device, {} ) )
//    //  , m_buffer( VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::exchange( rhs.m_buffer, {} ) )
//    //  , m_allocator( VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::exchange( rhs.m_allocator, {} ) )
//    //  , m_dispatcher( VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::exchange( rhs.m_dispatcher, nullptr ) )
//    //{
//    //}
//
//    Buffer& operator=(Buffer const&) = delete;
//
//    // to implement
//    //Buffer & operator=( Buffer && rhs ) noexcept
//    //{
//    //  if ( this != &rhs )
//    //  {
//    //    std::swap( m_device, rhs.m_device );
//    //    std::swap( m_buffer, rhs.m_buffer );
//    //    std::swap( m_allocator, rhs.m_allocator );
//    //    std::swap( m_dispatcher, rhs.m_dispatcher );
//    //  }
//    //  return *this;
//    //}
//
//    vk::Buffer const& operator*() const noexcept
//    {
//        return mBuffer;
//    }
//
//    void destroy() noexcept
//    {
//        //POLYPTODO("implement resource destroying");
//        //if ( m_buffer )
//        //{
//        //  getDispatcher()->vkDestroyBuffer(
//        //    static_cast<VkDevice>( m_device ), static_cast<VkBuffer>( m_buffer ), reinterpret_cast<const VkAllocationCallbacks *>( m_allocator ) );
//        //}
//        //m_device     = nullptr;
//        //m_buffer     = nullptr;
//        //m_allocator  = nullptr;
//        //m_dispatcher = nullptr;
//    }
//
//    [[nodiscard]] vk::MemoryRequirements getMemoryRequirements() const noexcept;
//
//private:
//    vk::Device   mDevice = {};
//    vk::Buffer   mBuffer = {};
//    VmaAllocator mVmaAllocator = {};
//};



//    VmaVulkanFunctions vulkanFunctions = {};
//    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
//    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
//
//    VmaAllocatorCreateInfo allocatorCreateInfo = {};
//    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
//    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
//    allocatorCreateInfo.physicalDevice = static_cast<VkPhysicalDevice>(*ctx.gpu());
//    allocatorCreateInfo.device = static_cast<VkDevice>(*device);
//    allocatorCreateInfo.instance = static_cast<VkInstance>(*ctx.instance());;
//    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
//    
//    auto vkres = vk::Result(vmaCreateAllocator(&allocatorCreateInfo, &mVmaAllocator));
//    if (vkres != vk::Result::eSuccess) {
//        POLYPFATAL("Failed to create VMA allocator.");
//    }
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