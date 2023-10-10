#ifndef RENDERER_H
#define RENDERER_H

#include "platforms.h"
#include "polyp_logs.h"

#include <memory>

namespace polyp {
namespace tools {

class IRenderer {
public:
    using Ptr = std::shared_ptr<IRenderer>;

    virtual ~IRenderer() = default;

    virtual bool onInit(WindowInstance inst, WindowHandle hwnd) = 0;
    virtual bool onResize() = 0;
    virtual void onMouseClick(uint32_t button, bool state) {
        POLYPDEBUG(__FUNCTION__);
    };
    virtual void onMouseMove(int x, int y) {
        POLYPDEBUG(__FUNCTION__);
    };
    virtual void onMouseWheel(float value) {
        POLYPDEBUG(__FUNCTION__);
    };
    virtual void onShoutDown() = 0;
    virtual bool isReady() = 0;
    virtual void draw() = 0;
    virtual void updateTimer() {
        POLYPDEBUG(__FUNCTION__);
    };
    virtual void mouseReset() {
        POLYPDEBUG(__FUNCTION__);
    };
};

} // tools
} // polyp

#endif // RENDERER_H
