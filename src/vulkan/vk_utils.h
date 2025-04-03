#pragma once

#include "vk_common.h"

namespace polyp {
namespace vulkan {
namespace utils {

template<typename CreateInfoT>
CreateInfoT getCreateInfo();

Fence createFence(bool signaled = false);

CommandBuffer createCommandBuffer(const CommandPool& pool, vk::CommandBufferLevel level);

std::tuple<Image, ImageView> createDepthStencil();

RenderPass createRenderPass();

Buffer createUploadBuffer(VkDeviceSize size);

Buffer createUploadBuffer(VkDeviceSize size, vk::BufferUsageFlags flags, VkMemoryPropertyFlags requiredFlags = 0);

Buffer createDeviceBuffer(VkDeviceSize size, vk::BufferUsageFlags flags, VkMemoryPropertyFlags requiredFlags = 0);

ShaderModule loadSPIRV(std::string path);

}
}
}
