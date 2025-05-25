#include <example_a.h>

using namespace polyp;
using namespace polyp::vulkan;

namespace polyp::vulkan {

class SimpleTriangle final: public example::ExampleA
{
protected:
    RHIContext::CreateInfo getRHICreateInfo() override
    {
        return utils::getCreateInfo<RHIContext::CreateInfo>();
    }

    ShaderData loadShaders() override
    {
        ShaderData output{};

        output.vertex   = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.vert.spv");
        output.fragment = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.frag.spv");

        return output;
    }

    UploadModelData loadModel() override
    {
        UploadModelData output{};

        std::vector<Vertex> vertexData =
        {
            { {  0.6f,  0.6f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { { -0.6f,  0.6f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  0.0f, -0.6f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
        };

        std::vector<uint32_t> indexData = { 0, 1, 2 };

        output.indexCount = indexData.size();

        const VkDeviceSize vertexBufferSize = vertexData.size() * sizeof(decltype(vertexData)::value_type);
        const VkDeviceSize indexBufferSize  = indexData.size()  * sizeof(decltype(indexData)::value_type);

        output.vertex = utils::createUploadBuffer(vertexBufferSize);
        output.index  = utils::createUploadBuffer(indexBufferSize);

        if (*output.vertex  == VK_NULL_HANDLE || *output.index   == VK_NULL_HANDLE)
        {
            POLYPFATAL("Failed to create upload buffers.");
            return {};
        }

        output.vertex.fill(vertexData);
        output.index.fill(indexData);

        return output;
    }

    UploadTextureData loadTexture() override
    {
        return {};
    }
};

} // namespace polyp::vulkan

int main()
{
    RUN_APP_EXAMPLE(SimpleTriangle);

    return EXIT_SUCCESS;
}
