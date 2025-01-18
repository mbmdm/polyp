#pragma once

#include "example_base.h"
#include "vk_utils.h"

namespace polyp {
namespace vulkan {
namespace example {

class ExampleA : public ExampleBase
{
protected:
    struct Vertex
    {
        float position[3];
        float color[3];
    };

    void                     draw()             override;
    bool                     postInit()         override;
    bool                     postResize()       override;
    RHIContext::CreateInfo   getRHICreateInfo() override;

    void                     updateUniformBuffer();

    using ShadersData = std::tuple<ShaderModule/*vert*/, ShaderModule/*frag*/>;
    using ModelsData  = std::tuple<std::vector<Vertex>/*vertices*/, std::vector<uint32_t>/*indexes*/>;

    virtual ShadersData      loadShaders() = 0;
    virtual ModelsData       loadModel()   = 0;

    CommandBuffer            mTransferCmd    = { VK_NULL_HANDLE };
    Buffer                   mVertexBuffer   = { VK_NULL_HANDLE };
    Buffer                   mIndexBuffer    = { VK_NULL_HANDLE };
    Buffer                   mUniformBuffer  = { VK_NULL_HANDLE };
    DescriptorSetLayout      mDSLayout       = { VK_NULL_HANDLE };
    PipelineLayout           mPipelineLayout = { VK_NULL_HANDLE };
    DescriptorPool           mDesriptorPool  = { VK_NULL_HANDLE };
    DescriptorSet            mDescriptorSet  = { VK_NULL_HANDLE };
    RenderPass               mRenderPass     = { VK_NULL_HANDLE };
    Pipeline                 mPipeline       = { VK_NULL_HANDLE };
    std::vector<Framebuffer> mFrameBuffers   = {};
    std::vector<Vertex>      mVertexData     = {};
    std::vector<uint32_t>    mIndexData      = {};

    struct
    {
        vulkan::Image    image = VK_NULL_HANDLE;
        vulkan::ImageView view = VK_NULL_HANDLE;
    } mDepthStencil;

private:
    void createBuffers();
    void createLayouts();
    void createDS();
    void createPipeline();
    void prepareDrawCommands();
};

} // example
} // vulkan
} // polyp
