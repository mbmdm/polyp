#ifndef WINDOW_SURFACE_H
#define WINDOW_SURFACE_H

#include "interfaces/renderer.h"
#include "platforms.h"

#include <memory>

namespace polyp {
namespace tools {

class WindowSurface {
public:
    WindowSurface(const char* title,
                  int x, int y, int width, int height, 
                  IRenderer::Ptr renderer);
    
    virtual ~WindowSurface();

    void run();

private:
    struct {
        WindowsInstance inst;
        WindowsHandle   hwnd;
    } mWindowHandle;
    IRenderer::Ptr mRenderer;
    bool mInitialized;
};

} // utils
} // polyp

#endif // WINDOW_SURFACE_H
