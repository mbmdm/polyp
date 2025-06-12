#include <example_a.h>
#include <model_loader.h>
#include <image_loader.h>

using namespace polyp;
using namespace polyp::vulkan;

namespace polyp::vulkan {

class SimpleTexture final : public example::ExampleA
{
protected:
    RHIContext::CreateInfo getRHICreateInfo() override
    {
        return utils::getCreateInfo<RHIContext::CreateInfo>();
    }

    ShaderData loadShaders() override
    {
        ShaderData output{};

        output.vertex   = utils::loadSPIRV("shaders/simple_texture/simple_texture.vert.spv");
        output.fragment = utils::loadSPIRV("shaders/simple_texture/simple_texture.frag.spv");

        return output;
    }

    UploadModelData loadModel() override
    {
        UploadModelData output{};

        std::string path = std::string(POLYP_ASSETS_LOCATION) + "models/rock.obj";

        auto loader = polyp::ModelLoader::load(path);

        if (std::string msg; loader.empty() && loader.hasError(msg))
            POLYPFATAL("%s", msg.c_str());
        else if (std::string msg; loader.hasError(msg))
            POLYPWARN("%s", msg.c_str());

        mCamera.reset(loader.lookPosition(), loader.center());

        std::vector<glm::vec3> positions = loader.positions();
        std::vector<uint32_t>  indexData = loader.indices();
        std::vector<glm::vec2> texCoords = loader.texCoords();

        std::vector<Vertex> vertexData(positions.size());

        float defaultColor[3] = { 1.0f, 1.0f, 1.0f };

        for (size_t i = 0; i < vertexData.size(); ++i)
        {
            memcpy_s(vertexData[i].position, sizeof(Vertex::position),
                     &positions[i],          sizeof(glm::vec3));
            memcpy_s(vertexData[i].color,    sizeof(Vertex::color),
                     defaultColor,           sizeof(defaultColor));
            memcpy_s(vertexData[i].texCoord, sizeof(Vertex::texCoord),
                     &texCoords[i],          sizeof(glm::vec2));
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

    UploadTextureData loadTexture() override
    {
        UploadTextureData output{};

        std::string path = std::string(POLYP_ASSETS_LOCATION) + "textures/rock.png";

        auto loader = polyp::ImageLoader::load(path);

        if (std::string msg; loader.empty() && loader.hasError(msg))
            POLYPFATAL("%s", msg.c_str());
        else if (std::string msg; loader.hasError(msg))
            POLYPWARN("%s", msg.c_str());

        output.width    = loader.width();
        output.height   = loader.height();
        output.channels = loader.channels();
        output.texture  = utils::createUploadBuffer(loader.size());

        output.texture.fill(static_cast<const void*>(loader.data()), loader.size());

        return output;
    }
};

} // namespace polyp::vulkan

int main(int argc, char* argv[])
{
    RUN_APP_EXAMPLE(SimpleTexture);

    return EXIT_SUCCESS;
}
