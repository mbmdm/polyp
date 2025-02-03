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

    ShadersData loadShaders() override
    {
        auto vert  = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.vert.spv");
        auto index = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.frag.spv");

        return std::make_tuple(std::move(vert), std::move(index));
    }

    ModelsData loadModel() override
    {
        std::vector<Vertex> vertexData =
        {
            { {  0.6f,  0.6f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { { -0.6f,  0.6f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { {  0.0f, -0.6f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
        };

        std::vector<uint32_t> indexData = { 0, 1, 2 };

        return std::make_tuple(std::move(vertexData), std::move(indexData));
    }
};

} // namespace polyp::vulkan

int main()
{
    RUN_APP_EXAMPLE(SimpleTriangle);

    return EXIT_SUCCESS;
}
