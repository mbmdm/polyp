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

    ShadersData loadShaders() override
    {
        auto vert  = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.vert.spv");
        auto index = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.frag.spv");

        return std::make_tuple(std::move(vert), std::move(index));
    }

    ModelsData loadModel() override
    {
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
        std::vector<uint32_t>  indices   = loader.indices();

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

        return std::make_tuple(std::move(vertexData), std::move(indices));
    }
};

} // namespace polyp::vulkan

int main(int argc, char* argv[])
{
    if (argc > 1)
        gModelPath = argv[1];

    RUN_APP_EXAMPLE(LoadObjModel);

    return EXIT_SUCCESS;
}
