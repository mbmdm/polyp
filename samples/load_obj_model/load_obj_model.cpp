#include <example_a.h>
#include <model_loader.h>

using namespace polyp;
using namespace polyp::vulkan;

std::string gModelPath = "";

namespace polyp::vulkan {

class LoadObjModel final : public example::ExampleA
{
public:
    LoadObjModel()
    {
        mRenderOptions.solid = false;
    }

protected:
    RHIContext::CreateInfo getRHICreateInfo() override
    {
        auto info = utils::getCreateInfo<RHIContext::CreateInfo>();
        info.device.features.fillModeNonSolid = true;
        return info;
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

        std::string path = gModelPath;
        if (path.empty())
            path = std::string(POLYP_ASSETS_LOCATION) + "models/wuson.obj";

        auto loader = polyp::ModelLoader::load(path);

        if (std::string msg; loader.empty() && loader.hasError(msg))
            POLYPFATAL("%s", msg.c_str());
        else if (std::string msg; loader.hasError(msg))
            POLYPWARN("%s", msg.c_str());

        mCamera.reset(loader.lookPosition(), loader.center());

        std::vector<glm::vec3> positions = loader.positions();
        std::vector<uint32_t>  indexData = loader.indices();

        std::vector<Vertex> vertexData(positions.size());

        float defaultColor[3] = { 1.0f, 1.0f, 1.0f };

        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            vertexData[i].position[0] = positions[i].x;
            vertexData[i].position[1] = positions[i].y;
            vertexData[i].position[2] = positions[i].z;
            memcpy_s(vertexData[i].color, sizeof(vertexData[i].color),
                     defaultColor,        sizeof(defaultColor));
        }

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
};

} // namespace polyp::vulkan

int main(int argc, char* argv[])
{
    if (argc > 1)
        gModelPath = argv[1];
    else
        POLYPINFO("Sample is able to load any OBJ-format model. "
                  "Specify the path to the model as a command-line argument.");


    RUN_APP_EXAMPLE(LoadObjModel);

    return EXIT_SUCCESS;
}
