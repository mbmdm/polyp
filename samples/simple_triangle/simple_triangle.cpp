#include <example_2D.h>

using namespace polyp;

using ShaderModule = polyp::vulkan::ShaderModule;

class SimpleTriangle : public example::Example2D
{
protected:
    std::tuple<ShaderModule/*vert*/, ShaderModule/*frag*/> loadShaders() override
    {
        auto vert  = vulkan::utils::loadSPIRV("shaders/simple_triangle/simple_triangle.vert.spv");
        auto index = vulkan::utils::loadSPIRV("shaders/simple_triangle/simple_triangle.frag.spv");
        
        return std::make_tuple(std::move(vert), std::move(index));
    }
};

int main()
{
    std::string title{ constants::kWindowTitle };
    title += ": simple triangle";

    SimpleTriangle sample{};

    Application::get().onWindowInitialized += [&sample](const auto& args) {sample.onInit(args); };
    Application::get().onWindowResized     += [&sample](const auto& args) {sample.onResize(); };
    Application::get().onFrameRender       += [&sample]() {sample.draw(); };

    Application::get().init(title.c_str(), 1024, 600);
    Application::get().run();

    return EXIT_SUCCESS;
}
