#pragma once

#include "event.h"

#ifdef WIN32
#include <Windows.h>
#endif

namespace polyp {

enum class MouseButton
{
    Left,
    Right
};

enum class MouseActioin
{
    Click,
    Release
};

struct WindowInitializedEventArgs
{
    HWND        windowHandle = NULL;
    HINSTANCE windowInstance = NULL;
};

struct WindowResizeEventArgs
{
    uint32_t width;
    uint32_t height;
};

struct MouseClickEventArgs
{
    MouseButton  button;
    MouseActioin action;
};

struct MouseMoveEventArgs
{
    float x;
    float y;
};

struct MouseWheelEventArgs : public MouseMoveEventArgs
{
    float delta;
};

class Application final
{
public:
    Event<void(const WindowInitializedEventArgs&)> onWindowInitialized;
    Event<void(const WindowResizeEventArgs&)>      onWindowResized;
    Event<void(const MouseClickEventArgs&)>        onMouseClick;
    Event<void(const MouseMoveEventArgs&)>         onMouseMove;
    Event<void(const MouseWheelEventArgs&)>        onMouseWheel;
    Event<void()>                                  onFrameRender;
    Event<void()>                                  onShutdown;

    static Application& get()
    {
        static Application instance;
        return instance;
    }

    bool init(const char* title, int width, int height);

    void run();

private:
    Application() : mWindowHandle(NULL), mWindowInstance(NULL)
    { }

    ~Application() { destroyWindow(); }

    void destroyWindow();

    HWND        mWindowHandle;
    HINSTANCE mWindowInstance;
};

}