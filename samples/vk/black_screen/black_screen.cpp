#include <example_raii.h>
#include <application.h>

using namespace polyp;

class BlackScreenRAII : public example::ExampleBaseRAII
{
public:
    virtual void draw() override {
        preDraw();
        postDraw();
    }
};

int main()
{
    std::string title{ constants::kWindowTitle };
    title += ": black screen";

    BlackScreenRAII sample{};

    Application::get().onWindowInitialized += [&sample](const WindowInitializedEventArgs& args) { sample.onInit(args.windowInstance, args.windowHandle); };
    Application::get().onWindowResized     += [&sample](const auto& args) {sample.onResize(); };
    Application::get().onFrameRender       += [&sample]() {sample.draw(); };

    Application::get().init(title.c_str(), 1024, 600);
    Application::get().run();
}