#ifndef RENDERER_H
#define RENDERER_H

#include "platforms.h"

#include <memory>

namespace polyp {
namespace tools {

class IRenderer {
public:
    using Ptr = std::shared_ptr<IRenderer>;

    virtual ~IRenderer() = default;

    virtual bool onInit() = 0;
    virtual bool onResize() = 0;
    virtual void onMouseClick(uint32_t button, bool state) = 0;
    virtual void onMouseMove(int x, int y) = 0;
    virtual void onMouseWheel(float value) = 0;
    virtual void onShoutDown() = 0;

    virtual bool isReady() = 0;
    virtual void draw() = 0;
    virtual void updateTimer() = 0;
    virtual void mouseReset() = 0;
};

} // tools
} // polyp

#endif // RENDERER_H
