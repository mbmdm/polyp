#include <example_2D.h>

using namespace polyp;

int main()
{
    std::string title{ constants::kWindowTitle };
    title += ": simple triangle";

    example::Example2D sample{};

    Application::get().onWindowInitialized += [&sample](const auto& args) {sample.onInit(args); };
    Application::get().onWindowResized     += [&sample](const auto& args) {sample.onResize(); };
    Application::get().onFrameRender       += [&sample]() {sample.draw(); };

    Application::get().init(title.c_str(), 1024, 600);
    Application::get().run();

    return EXIT_SUCCESS;
}
