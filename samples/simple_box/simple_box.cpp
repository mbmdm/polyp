#include <example_a.h>

using namespace polyp;
using namespace polyp::vulkan;

namespace polyp::vulkan {

class SimpleBox : public example::ExampleA
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
    
        std::vector<Vertex> vertexData(sizeof(vertices) / (sizeof(float) * 6));
    
        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            size_t j = i * 6;
    
            vertexData[i].position[0] = vertices[j];
            vertexData[i].position[1] = vertices[j + 1];
            vertexData[i].position[2] = vertices[j + 2];
    
            j += 3;
    
            vertexData[i].color[0] = vertices[j];
            vertexData[i].color[1] = vertices[j + 1];
            vertexData[i].color[2] = vertices[j + 2];
        }
    
        std::vector<uint32_t> indexData(vertexData.size());
    
        std::iota(indexData.begin(), indexData.end(), 0);
    
        return std::make_tuple(std::move(vertexData), std::move(indexData));
    }
};

} // namespace namespace polyp::vulkan

int main()
{
    RUN_APP_EXAMPLE(SimpleBox);

    return EXIT_SUCCESS;
}
