#include <example_base.h>
#include <vk_utils.h>

using namespace polyp;
using namespace polyp::vulkan;

namespace polyp::vulkan {

class BlackScreen final : public example::ExampleBase 
{
protected:
    bool postResize() override { return true; }

    bool postInit() override { return true; }

    void draw() override
    {
        RHIContext::get().device().waitIdle(); // Simply make sure that the cmd is not in a pending stage

        CommandBuffer& cmd = mDrawCmds[mCurrSwImIndex];

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        cmd.begin(beginInfo);

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
        
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,
                            vk::PipelineStageFlagBits::eBottomOfPipe,
                            vk::DependencyFlags(), nullptr, nullptr, barrier);

        cmd.end();
    }

    RHIContext::CreateInfo getRHICreateInfo() override
    {
        return utils::getCreateInfo<RHIContext::CreateInfo>();
    }
};

}

int main()
{
    RUN_APP_EXAMPLE(BlackScreen);

    return EXIT_SUCCESS;
}