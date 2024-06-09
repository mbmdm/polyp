#ifndef EXAMPLEBASICPIPELINE_H
#define EXAMPLEBASICPIPELINE_H

#include "example_base.h"
#include "vk_utils.h"

namespace polyp {
namespace vulkan {
namespace example {

class ExampleBasicPipeline : public ExampleBase
{
public:
    ExampleBasicPipeline();

    struct Vertex
    {
        float position[3];
        float color[3];
    };

    struct UniformData
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

    std::vector<Vertex>   mVertexData      = {};
    std::vector<uint32_t> mIndexData       = {};
    UniformData           mUniformData     = {};

    using ShadersData = std::tuple<ShaderModule/*vert*/, ShaderModule/*frag*/>;
    using ModelsData  = std::tuple<std::vector<Vertex>/*vertices*/, std::vector<uint32_t>/*indexes*/>;

    virtual ShadersData loadShaders() = 0;
    virtual ModelsData  loadModel();
    virtual UniformData loadUniformData();

private:
    void createTransferCmd();
    void createBuffers();
    void createLayouts();
    void createDS();
    void createPipeline();
    void render();

public:
    bool onInit(const WindowInitializedEventArgs& args) override;
    void onNextFrame() override;
};

} // example
} // vulkan
} // polyp

#endif // EXAMPLEBASICPIPELINE_H
