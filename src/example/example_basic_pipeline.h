#ifndef EXAMPLE2D_H
#define EXAMPLE2D_H

#include "example_base.h"
#include "vk_utils.h"

namespace polyp {
namespace example {

using namespace polyp::vulkan;

class ExampleBasicPipeline : public ExampleBase
{
public:
    ExampleBasicPipeline();

    struct Vertex
    {
        float position[3];
        float color[3];
    };

    struct ShaderData
    {
        glm::mat4 projectionMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
    };

protected:
    CommandPool           mTransferCmdPool = { VK_NULL_HANDLE };
    CommandBuffer         mTransferCmd     = { VK_NULL_HANDLE };
    Buffer                mVertexBuffer    = { VK_NULL_HANDLE };
    Buffer                mIndexBuffer     = { VK_NULL_HANDLE };
    Buffer                mUniformBuffer   = { VK_NULL_HANDLE };
    DescriptorSetLayout   mDSLayout        = { VK_NULL_HANDLE };
    PipelineLayout        mPipelineLayout  = { VK_NULL_HANDLE };
    DescriptorPool        mDesriptorPool   = { VK_NULL_HANDLE };
    DescriptorSet         mDescriptorSet   = { VK_NULL_HANDLE };
    Pipeline              mPipeline        = { VK_NULL_HANDLE };

    std::vector<Vertex>   mVertexData 
    {
        { {  0.6f,  0.6f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -0.6f,  0.6f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.0f, -0.6f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };

    std::vector<uint32_t> mIndexData{ 0, 1, 2 };

    ShaderData            mShaderData
    {
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f)
    };

    virtual std::tuple<ShaderModule/*vert*/, ShaderModule/*frag*/> loadShaders() = 0;

private:
    void createTransferCmd();
    void createBuffers();
    void createLayouts();
    void createDS();
    void createPipeline();
    void render();

public:
    bool onInit(const WindowInitializedEventArgs& args) override;
    void draw() override;
};

} // example
} // polyp

#endif // EXAMPLE2D_H
