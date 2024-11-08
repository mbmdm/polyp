#include <example_base.h>
#include <vk_utils.h>

using namespace polyp;
using namespace polyp::vulkan;

namespace polyp::vulkan {

class BlackScreen final : public example::ExampleBase 
{
private:
    bool postResize() override { return true; }

    bool postInit() override
    {
        mCmdBuffer = utils::createCommandBuffer(mCmdPool, vk::CommandBufferLevel::ePrimary);
        if (*mCmdBuffer == VK_NULL_HANDLE) {
            return false;
        }

        POLYPDEBUG("Primary command buffer created successfully");

        return true;
    }

    SubmitInfo getSubmitCmd() override
    {
        RHIContext::get().device().waitIdle(); // Simply make sure that the cmd is not in a pending stage

        mCmdBuffer.reset();

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        mCmdBuffer.begin(beginInfo);

        auto barrier = vk::ImageMemoryBarrier{};
        
        barrier.image                           = mSwapChainImages[mCurrSwImIndex];
        barrier.srcAccessMask                   = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask                   = vk::AccessFlagBits::eMemoryRead;
        barrier.oldLayout                       = vk::ImageLayout::eUndefined;
        barrier.newLayout                       = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = VK_REMAINING_MIP_LEVELS;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = VK_REMAINING_ARRAY_LAYERS;
        
        mCmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::PipelineStageFlagBits::eTopOfPipe,
                                   vk::DependencyFlags(), nullptr, nullptr, barrier);

        mCmdBuffer.end();

        return SubmitInfo{ VK_NULL_HANDLE,{*mCmdBuffer} };
    }

    RHIContext::CreateInfo getRHICreateInfo() override
    {
        return utils::getCreateInfo<RHIContext::CreateInfo>();
    }

    CommandBuffer mCmdBuffer = { VK_NULL_HANDLE };
};

}

int main()
{
    RUN_APP_EXAMPLE(BlackScreen);

    return EXIT_SUCCESS;
}