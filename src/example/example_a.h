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

    struct ShaderData
    {
        ShaderModule vertex   = { VK_NULL_HANDLE };
        ShaderModule fragment = { VK_NULL_HANDLE };
    };

    struct UploadModelData
    {
        Buffer   vertex     = { VK_NULL_HANDLE };
        Buffer   index      = { VK_NULL_HANDLE };
        uint32_t indexCount = 0;
    };

    struct UploadTextureData
    {
        uint32_t width    = 0;
        uint32_t height   = 0;
        uint32_t channels = 0;
        Buffer   texture  = { VK_NULL_HANDLE };
    };

    struct DepthStencilData
    {
        vulkan::Image     image = VK_NULL_HANDLE;
        vulkan::ImageView view  = VK_NULL_HANDLE;
    };

    struct RenderOptions
    {
        bool solid = true;
    };

    void                     draw()             override;
    bool                     postInit()         override;
    bool                     postResize()       override;
    RHIContext::CreateInfo   getRHICreateInfo() override;

    void                     updateUniformBuffer();

    virtual ShaderData        loadShaders() = 0;
    virtual UploadModelData   loadModel()   = 0;
    virtual UploadTextureData loadTexture() = 0;

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
    DepthStencilData         mDepthStencil   = {};
    RenderOptions            mRenderOptions  = {};
    uint32_t                 mDrawIndexCount = 0;

private:
    void createBuffers(const UploadModelData& data);
    void createTextures(const UploadTextureData& data);
    void createLayouts();
    void createDS();
    void createPipeline();
    void prepareDrawCommands();
};

} // example
} // vulkan
} // polyp
