#include "vk_common.h"

namespace polyp {
namespace vulkan {
namespace utils {

std::tuple<Image, ImageView> createDepthStencil();

RenderPass createRenderPass();

Buffer createUploadBuffer(VkDeviceSize size);

Buffer createDeviceBuffer(VkDeviceSize size, vk::BufferUsageFlags flags);

ShaderModule loadSPIRV(std::string path);

}
}
}
