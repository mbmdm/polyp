#include <example_a.h>
#include <model_loader.h>

using namespace polyp;
using namespace polyp::vulkan;

namespace polyp::vulkan {

class LoadObjModel final : public example::ExampleA
{
public:
    LoadObjModel()
    {
        mCamera.processKeyboard(Camera::Direction::Up,      0.8f);
        mCamera.processKeyboard(Camera::Direction::Right,   3.1f);
        mCamera.processKeyboard(Camera::Direction::Forward, 0.7f);
        mCamera.procesMouse(-1.1, 0.0f, 1.0f);
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
        std::string path = std::string(POLYP_ASSETS_LOCATION) + "models/wuson.obj";
        auto modelLoader      = polyp::ModelLoader::loadModel(path);

        if (std::string msg; modelLoader.empty() && modelLoader.hasError(msg))
            POLYPFATAL("%s", msg.c_str());
        else if (std::string msg; modelLoader.hasError(msg))
            POLYPWARN("%s", msg.c_str());

        std::vector<Vertex> vertexData(modelLoader.positions().size());

        float defaultColor[3] = { 1.0f, 1.0f, 1.0f };

        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            vertexData[i].position[0] = modelLoader.positions()[i].x;
            vertexData[i].position[1] = modelLoader.positions()[i].y;
            vertexData[i].position[2] = modelLoader.positions()[i].z;
            memcpy_s(vertexData[i].color, sizeof(vertexData[i].color),
                     defaultColor,        sizeof(defaultColor));
        }

        return std::make_tuple(std::move(vertexData), modelLoader.indices());
    }
};

} // namespace polyp::vulkan

int main()
{
    RUN_APP_EXAMPLE(LoadObjModel);

    return EXIT_SUCCESS;
}
