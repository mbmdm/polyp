#include <example_raii.h>

using namespace polyp;

class BlackScreenRAII : public polyp::example::ExampleBaseRAII {
public:
    virtual void draw() override {
        preDraw();
        postDraw();
    }
};

int main() {
    std::string title{ constants::kWindowTitle };
    title += ": black screen";

    tools::IRenderer::Ptr sample = std::make_shared<BlackScreenRAII>();
    tools::PolypWindow win{title.c_str(), 0, 0, 1024, 600, sample};

    win.run();
}
