#include "vk_utils.h"
#include "vk_context.h"

#include <fstream>

namespace polyp {
namespace vulkan {
namespace utils {

template<>
RHIContext::CreateInfo getCreateInfo<RHIContext::CreateInfo>()
{
    using namespace constants;

    std::vector<RHIContext::CreateInfo::Queue> queInfos{
        {1, vk::QueueFlagBits::eGraphics, true}
    };

    return RHIContext::CreateInfo {
         {kInternalApplicationName, 1},         // CreateInfo::Application
         RHIContext::CreateInfo::GPU::Powerful, // CreateInfo::GPU
         {NULL, NULL},                          // CreateInfo::Surface
         {queInfos, {}},                        // CreateInfo::Device
         {3},                                   // CreateInfo::SwapChain
    };
}

Fence createFence(bool signaled)
{
    vk::FenceCreateInfo createInfo{};

    if (signaled)
        createInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    return RHIContext::get().device().createFence(createInfo);
}

CommandBuffer createCommandBuffer(const CommandPool& pool, vk::CommandBufferLevel level)
{
    const auto& device = RHIContext::get().device();

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool        = *pool;
    allocInfo.level              = level;
    allocInfo.commandBufferCount = 1;

    auto cmds = device.allocateCommandBuffers(allocInfo);
    if (!cmds.empty())
        return std::move(cmds[0]);

    POLYPERROR("Failed to allocate command buffer(s).");
    return VK_NULL_HANDLE;
}

std::tuple<Image, ImageView> createDepthStencil()
{
    const auto& gpu     = RHIContext::get().gpu();
    const auto& device  = RHIContext::get().device();
    const auto& surface = RHIContext::get().surface();

    Image    image = VK_NULL_HANDLE;
    ImageView view = VK_NULL_HANDLE;

    auto capabilities = gpu.getSurfaceCapabilitiesKHR(*surface);
    const auto width  = capabilities.currentExtent.width;
    const auto height = capabilities.currentExtent.height;

    ImageCreateInfo imCreateInfo{};
    imCreateInfo.imageType   = vk::ImageType::e2D;
    imCreateInfo.format      = gpu.getDepthFormatPLP();
    imCreateInfo.extent      = vk::Extent3D(width, height, 1);
    imCreateInfo.mipLevels   = 1;
    imCreateInfo.arrayLayers = 1;
    imCreateInfo.samples     = vk::SampleCountFlagBits::e1;
    imCreateInfo.tiling      = vk::ImageTiling::eOptimal;
    imCreateInfo.usage       = vk::ImageUsageFlagBits::eDepthStencilAttachment;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

    image = device.createImagePLP(imCreateInfo, allocCreateInfo);

    vk::ImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.viewType                        = vk::ImageViewType::e2D;
    viewCreateInfo.image                           = *image;
    viewCreateInfo.format                          = imCreateInfo.format;
    viewCreateInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eDepth;
    if (viewCreateInfo.format >= vk::Format::eD16UnormS8Uint) {
        viewCreateInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
    }
    viewCreateInfo.subresourceRange.baseMipLevel   = 0;
    viewCreateInfo.subresourceRange.levelCount     = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount     = 1;

    view = device.createImageView(viewCreateInfo);

    return std::make_tuple(std::move(image), std::move(view));
}

RenderPass createRenderPass()
{
    const auto& gpu       = RHIContext::get().gpu();
    const auto& device    = RHIContext::get().device();
    const auto& swapchain = RHIContext::get().swapchain();

    std::array<vk::AttachmentDescription, 2> attachments = {};

    // Color attachment
    attachments[0].format         = swapchain.getImageFormatPLP();
    attachments[0].samples        = vk::SampleCountFlagBits::e1;
    attachments[0].loadOp         = vk::AttachmentLoadOp::eClear;
    attachments[0].storeOp        = vk::AttachmentStoreOp::eStore;
    attachments[0].stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    attachments[0].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[0].initialLayout  = vk::ImageLayout::eUndefined;
    attachments[0].finalLayout    = vk::ImageLayout::ePresentSrcKHR;
    // Depth attachment
    attachments[1].format         = gpu.getDepthFormatPLP();
    attachments[1].samples        = vk::SampleCountFlagBits::e1;
    attachments[1].loadOp         = vk::AttachmentLoadOp::eClear;
    attachments[1].storeOp        = vk::AttachmentStoreOp::eStore;
    attachments[1].stencilLoadOp  = vk::AttachmentLoadOp::eClear;
    attachments[1].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachments[1].initialLayout  = vk::ImageLayout::eUndefined;
    attachments[1].finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::AttachmentReference colorReference = {};
    colorReference.attachment = 0;
    colorReference.layout     = vk::ImageLayout::eColorAttachmentOptimal;

    vk::AttachmentReference depthReference = {};
    depthReference.attachment = 1;
    depthReference.layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics;
    subpassDescription.colorAttachmentCount    = 1;
    subpassDescription.pColorAttachments       = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.inputAttachmentCount    = 0;
    subpassDescription.pInputAttachments       = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments    = nullptr;
    subpassDescription.pResolveAttachments     = nullptr;

    std::array<vk::SubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
    dependencies[0].dstStageMask    = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
    dependencies[0].srcAccessMask   = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    dependencies[0].dstAccessMask   = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;
    dependencies[0].dependencyFlags = {};

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass      = 0;
    dependencies[1].srcStageMask    = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[1].dstStageMask    = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[1].srcAccessMask   = vk::AccessFlags();
    dependencies[1].dstAccessMask   = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;
    dependencies[0].dependencyFlags = {};

    vk::RenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies   = dependencies.data();

    return device.createRenderPass(renderPassInfo);
}

Buffer createUploadBuffer(VkDeviceSize size)
{
    return createUploadBuffer(size, {}, 0);
}

Buffer createUploadBuffer(VkDeviceSize size, vk::BufferUsageFlags flags, VkMemoryPropertyFlags requiredFlags)
{
    if (size == 0)
        return VK_NULL_HANDLE;

    const auto& device = vulkan::RHIContext::get().device();

    BufferCreateInfo createInfo;
    createInfo.size  = size;
    createInfo.usage = vk::BufferUsageFlagBits::eTransferSrc | flags;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage         = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags         = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    allocCreateInfo.requiredFlags = requiredFlags;

    return device.createBufferPLP(createInfo, allocCreateInfo);
}

Buffer createDeviceBuffer(VkDeviceSize size, vk::BufferUsageFlags flags, VkMemoryPropertyFlags requiredFlags)
{
    if (size == 0)
        return VK_NULL_HANDLE;

    const auto& device = vulkan::RHIContext::get().device();

    BufferCreateInfo createInfo;
    createInfo.size  = size;
    createInfo.usage = flags;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage         = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocCreateInfo.requiredFlags = requiredFlags;

    return device.createBufferPLP(createInfo, allocCreateInfo);
}

ShaderModule loadSPIRV(std::string path)
{
    std::ifstream is(path, std::ios::binary | std::ios::in | std::ios::ate);

    size_t size = 0;
    std::unique_ptr<char> code{ nullptr };

    if (is.is_open())
    {
        size = is.tellg();
        is.seekg(0, std::ios::beg);
        code.reset(new char[size]);
        is.read(code.get(), size);
        is.close();
        POLYPASSERT(size > 0 && "Load SPIR-V shared failed");
    }

    if (!code) {
        POLYPERROR("Failed to load SPIR-V file");
        return VK_NULL_HANDLE;
    }

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = size;
    createInfo.pCode    = (uint32_t*)code.get();

    return RHIContext::get().device().createShaderModule(createInfo);
};

}
}
}
