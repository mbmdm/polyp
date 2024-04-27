#pragma once

#include "common.h"

#include <vulkan/vulkan_raii.hpp>

namespace polyp {
namespace vulkan {

using namespace vk;

using Context        = vk::raii::Context;
//using Instance       = vk::raii::Instance;
using Device         = vk::raii::Device;
//using PhysicalDevice = vk::raii::PhysicalDevice;
using SurfaceKHR     = vk::raii::SurfaceKHR;
using SwapchainKHR   = vk::raii::SwapchainKHR;

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

    bool isDiscretePLP() const;

    uint64_t getPerformanceRatioPLP() const;

    bool supportPLP(const SurfaceKHR& surface, PresentModeKHR mode);
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



}
}