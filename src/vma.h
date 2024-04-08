#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>

namespace polyp {
namespace vulkan {

    class Buffer
    {
    //public:
    //  using CType   = VkBuffer;
    //  using CppType = vk::Buffer;

      //static VULKAN_HPP_CONST_OR_CONSTEXPR VULKAN_HPP_NAMESPACE::ObjectType objectType = VULKAN_HPP_NAMESPACE::ObjectType::eBuffer;
      //static VULKAN_HPP_CONST_OR_CONSTEXPR VULKAN_HPP_NAMESPACE::DebugReportObjectTypeEXT debugReportObjectType = VULKAN_HPP_NAMESPACE::DebugReportObjectTypeEXT::eBuffer;

    public:
      Buffer( std::nullptr_t ) {}

      ~Buffer()
      {
          destroy();
      }

      Buffer()                 = delete;
      Buffer( Buffer const & ) = delete;

      //need to implement
      //Buffer( Buffer && rhs ) noexcept
      //  : m_device( VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::exchange( rhs.m_device, {} ) )
      //  , m_buffer( VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::exchange( rhs.m_buffer, {} ) )
      //  , m_allocator( VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::exchange( rhs.m_allocator, {} ) )
      //  , m_dispatcher( VULKAN_HPP_NAMESPACE::VULKAN_HPP_RAII_NAMESPACE::exchange( rhs.m_dispatcher, nullptr ) )
      //{
      //}

      Buffer & operator=( Buffer const & ) = delete;

      // to implement
      //Buffer & operator=( Buffer && rhs ) noexcept
      //{
      //  if ( this != &rhs )
      //  {
      //    std::swap( m_device, rhs.m_device );
      //    std::swap( m_buffer, rhs.m_buffer );
      //    std::swap( m_allocator, rhs.m_allocator );
      //    std::swap( m_dispatcher, rhs.m_dispatcher );
      //  }
      //  return *this;
      //}

      vk::Buffer const & operator*() const noexcept
      {
        return mBuffer;
      }

      void destroy() noexcept
      {
        //POLYPTODO("implement resource destroying");
        //if ( m_buffer )
        //{
        //  getDispatcher()->vkDestroyBuffer(
        //    static_cast<VkDevice>( m_device ), static_cast<VkBuffer>( m_buffer ), reinterpret_cast<const VkAllocationCallbacks *>( m_allocator ) );
        //}
        //m_device     = nullptr;
        //m_buffer     = nullptr;
        //m_allocator  = nullptr;
        //m_dispatcher = nullptr;
      }

      [[nodiscard]] vk::MemoryRequirements getMemoryRequirements() const noexcept;

    private:
        vk::Device   mDevice       = {};
        vk::Buffer   mBuffer       = {};
        VmaAllocator mVmaAllocator = {};
    };
}
}
