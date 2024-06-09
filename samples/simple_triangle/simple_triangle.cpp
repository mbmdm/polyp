#include <example_basic_pipeline.h>

using namespace polyp;
using namespace polyp::vulkan;

class SimpleTriangle : public example::ExampleBasicPipeline
{
protected:
    ShadersData loadShaders() override
    {
        auto vert  = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.vert.spv");
        auto index = utils::loadSPIRV("shaders/simple_triangle/simple_triangle.frag.spv");
        
        return std::make_tuple(std::move(vert), std::move(index));
    }
};

int main()
{
    std::string title{ constants::kWindowTitle };
    title += ": simple triangle";

    SimpleTriangle sample{};

    Application::get().onWindowInitialized += [&sample](const auto& args) { sample.onInit(args); };
    Application::get().onWindowResized     += [&sample](const auto& args) { sample.onResize(args); };
    Application::get().onNextFrame         += [&sample]()                 { sample.onNextFrame(); };

    Application::get().init(title.c_str(), 1024, 600);
    Application::get().run();

    return EXIT_SUCCESS;
}
