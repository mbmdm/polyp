#include <example_a.h>

#include <random>

using namespace polyp;
using namespace polyp::vulkan;

namespace polyp::vulkan {

class SimpleBox final : public example::ExampleA
{
public:
    SimpleBox()
    {
        glm::vec3 pos = constants::kCameraInitPos;
        pos.z += mDeviation * 5;
        mCamera.position(pos);
        generatePositions();
    }

protected:
    RHIContext::CreateInfo getRHICreateInfo() override
    {
        return utils::getCreateInfo<RHIContext::CreateInfo>();
    }

    ShadersData loadShaders() override
    {
        auto vert  = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.vert.spv");
        auto index = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.frag.spv");

        return std::make_tuple(std::move(vert), std::move(index));
    }

    ModelsData loadModel() override
    {
        float vertices[] =
        {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.5f, 0.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.5f, 0.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f
        };

        std::vector<Vertex> cubeVertices(sizeof(vertices) / (sizeof(float) * 6));

        for (size_t i = 0; i < cubeVertices.size(); ++i)
        {
            size_t j = i * 6;

            cubeVertices[i].position[0] = vertices[j];
            cubeVertices[i].position[1] = vertices[j + 1];
            cubeVertices[i].position[2] = vertices[j + 2];

            j += 3;

            cubeVertices[i].color[0] = vertices[j];
            cubeVertices[i].color[1] = vertices[j + 1];
            cubeVertices[i].color[2] = vertices[j + 2];
        }

        std::vector<uint32_t> indexData(cubeVertices.size());

        std::iota(indexData.begin(), indexData.end(), 0);

        std::vector<Vertex> vertexData;
        for (size_t i = 0; i < mBoxCount; ++i)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, mBoxPositions[i]);
            float angle     = rand() % 60;
            model           = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

            for (size_t v = 0; v < cubeVertices.size(); ++v)
            {
                auto data = cubeVertices[v];
                glm::vec4 transformed = model * glm::vec4(data.position[0], data.position[1], data.position[2], 1.0);

                data.position[0] = transformed.x;
                data.position[1] = transformed.y;
                data.position[2] = transformed.z;

                vertexData.push_back(data);
            }
        }

        POLYPASSERT((vertexData.size() % mBoxCount) == 0 &&
                    (vertexData.size() / mBoxCount) == indexData.size());

        return std::make_tuple(std::move(vertexData), std::move(indexData));
    }

    void draw() override
    {
        CommandBuffer& cmd = mDrawCmds[mCurrSwImIndex];

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        const auto& ctx = RHIContext::get();
        auto capabilities = ctx.gpu().getSurfaceCapabilitiesKHR(*ctx.surface());

        const uint32_t width  = capabilities.currentExtent.width;
        const uint32_t height = capabilities.currentExtent.height;

        vk::ClearValue clearValues[2];
        clearValues[0].color        = vk::ClearColorValue{ 0.4f, 0.4f, 0.4f, 1.0f };
        clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

        vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.renderPass               = *mRenderPass;
        renderPassBeginInfo.renderArea.offset.x      = 0;
        renderPassBeginInfo.renderArea.offset.y      = 0;
        renderPassBeginInfo.renderArea.extent.width  = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        renderPassBeginInfo.clearValueCount          = 2;
        renderPassBeginInfo.pClearValues             = clearValues;
        renderPassBeginInfo.framebuffer              = *mFrameBuffers[mCurrSwImIndex];

        cmd.begin(beginInfo);
        cmd.beginRenderPass(renderPassBeginInfo, SubpassContents::eInline);

        vk::Viewport viewport{};
        viewport.height   = (float)height;
        viewport.width    = (float)width;
        viewport.minDepth = (float)0.0f;
        viewport.maxDepth = (float)1.0f;

        // View port transformation flipping (like in OpenGL)
        //viewport.height = -(float)height;
        //viewport.width  = (float)width;
        //viewport.x      = 0;
        //viewport.y      = height;

        std::vector<vk::Viewport> viewpors{ viewport };
        cmd.setViewport(0, viewpors);

        vk::Rect2D scissor{};
        scissor.extent.width  = width;
        scissor.extent.height = height;
        scissor.offset.x      = 0;
        scissor.offset.y      = 0;

        std::vector<vk::Rect2D> scissors{ scissor };
        cmd.setScissor(0, scissors);

        std::vector<uint32_t> dynamicOffsets{ static_cast<uint32_t>(sizeof(MVP)) * mCurrSwImIndex };
        VkDeviceSize verBufferOffset = 0;

        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, { *mDescriptorSet }, dynamicOffsets);
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *mPipeline);
        cmd.bindIndexBuffer(*mIndexBuffer, 0, vk::IndexType::eUint32);
        cmd.bindVertexBuffers(0, { *mVertexBuffer }, { verBufferOffset });

        const size_t boxesCount = mBoxPositions.size();
        for (size_t i = 0; i < boxesCount; i++)
        {
            VkDeviceSize offset = ((mVertexData.size() / boxesCount) * sizeof(decltype(mVertexData)::value_type)) * i;
            cmd.bindVertexBuffers(0, { *mVertexBuffer }, { offset });
            cmd.drawIndexed(mIndexData.size(), 1, 0, 0, 1);
        }

        cmd.endRenderPass();
        cmd.end();

        example::ExampleA::updateUniformBuffer();
    }

private:
    void generatePositions()
    {
        mBoxPositions.resize(mBoxCount);

        std::vector<int> rands(mBoxCount * 3, 0);

        std::random_device rd;
        std::mt19937 gen(rd());

        std::normal_distribution<float> distribution(0.0f, mDeviation);

        for (size_t i = 0; i < mBoxCount; ++i)
        {
            mBoxPositions[i].x = distribution(gen);
            mBoxPositions[i].y = distribution(gen);
            mBoxPositions[i].z = distribution(gen);
        }
    }

    std::vector<glm::vec3> mBoxPositions;
    const uint32_t mBoxCount = 1000;
    const float mDeviation = 5.0f;
};

} // namespace polyp::vulkan

int main()
{
    RUN_APP_EXAMPLE(SimpleBox);

    return EXIT_SUCCESS;
}
