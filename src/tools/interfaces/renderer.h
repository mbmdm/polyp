#ifndef RENDERER_H
#define RENDERER_H

#include "platforms.h"

#include <memory>

namespace polyp {
namespace tools {

class IRenderer {
public:
    using Ptr = std::shared_ptr<IRenderer>;

    virtual ~IRenderer() = 0;

    virtual void onInit() = 0;
    virtual void onResize() = 0;
    virtual void onMouseClick(uint32_t button, bool state) = 0;
    virtual void onMouseMove(int x, int y) = 0;
    virtual void onMouseWheel(float value) = 0;
    virtual void onMuseReset() = 0;
    virtual void onUpdateTimer() = 0;
    virtual void onShoutDown() = 0;
};

} // tools
} // polyp

#endif // RENDERER_H
