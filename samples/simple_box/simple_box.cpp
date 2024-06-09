#include <example_basic_pipeline.h>

using namespace polyp;
using namespace polyp::vulkan;

class SimpleBox : public example::ExampleBasicPipeline
{
protected:
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

    UniformData loadUniformData() override
    {
        glm::mat4 view       = glm::mat4(1.0f);
        glm::mat4 model      = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        auto& ctx = vulkan::RHIContext::get();
        auto capabilities = ctx.gpu().getSurfaceCapabilitiesKHR(*ctx.surface());

        auto width  = capabilities.currentExtent.width;
        auto height = capabilities.currentExtent.height;

        view = glm::lookAt(glm::vec3(1.0f, 3.0f, 3.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, 1.0f, 0.0f));

        projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

        return UniformData{ projection, model, view };
    }
};

int main()
{
    std::string title{ constants::kWindowTitle };
    title += ": simple box";

    SimpleBox sample{};

    Application::get().onWindowInitialized += [&sample](const auto& args) { sample.onInit(args); };
    Application::get().onWindowResized     += [&sample](const auto& args) { sample.onResize(args);};
    Application::get().onNextFrame         += [&sample]()                 { sample.onNextFrame(); };

    Application::get().init(title.c_str(), 1024, 600);
    Application::get().run();

    return EXIT_SUCCESS;
}
