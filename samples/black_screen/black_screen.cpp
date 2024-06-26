#include <example_base.h>

using namespace polyp;

int main()
{
    std::string title{ constants::kWindowTitle };
    title += ": black screen";

    example::ExampleBase sample{};

    Application::get().onWindowInitialized += [&sample](const auto& args) {sample.onInit(args); };
    Application::get().onWindowResized     += [&sample](const auto& args) {sample.onResize(args); };
    Application::get().onFrameRender       += [&sample]() {sample.draw(); };

    Application::get().init(title.c_str(), 1024, 600);
    Application::get().run();

    return EXIT_SUCCESS;
}