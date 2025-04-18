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

enum class WindowResizeMode
{
    Restored = 0,
    Minimized,
    Maxshow,
    Maximized,
    Maxide,
    ExitSizeMove = 100
};

struct WindowInitializedEventArgs
{
    HWND        windowHandle = NULL;
    HINSTANCE windowInstance = NULL;
};

struct WindowResizeEventArgs
{
    WindowResizeMode  mode;
    uint32_t         width;
    uint32_t        height;
};

struct MouseClickEventArgs
{
    MouseButton  button;
    MouseActioin action;
};

struct MovementEventArgs
{
    struct
    {
        bool ahead = false;
        bool back  = false;
        bool righ  = false;
        bool left  = false;
        bool up    = false;
        bool down  = false;
    } move;

    struct
    {
        float x = 0;
        float y = 0;
        float wheel = 0;

    } mouse;

    bool reset = false;

    bool HasMotion() const
    {
        return move.ahead || move.back || move.righ || move.left || move.up || move.down;
    }

    bool HasMouse() const
    {
        return mouse.x != 0 && mouse.y != 0;
    }

    bool HasWheel() const
    {
        return mouse.wheel != 0;
    }

    bool HasReset() const
    {
        return reset;
    }
};

class Application final
{
public:
    Event<void(const WindowInitializedEventArgs&)> onWindowInitialized;
    Event<void(const WindowResizeEventArgs&)>      onWindowResized;
    Event<void(const MouseClickEventArgs&)>        onMouseClick;
    Event<void(const MovementEventArgs&)>          onMovement;
    Event<void()>                                  onShutdown;
    Event<void()>                                  onRender;

    static Application& get()
    {
        static Application instance;
        return instance;
    }

    bool init(const char* title, int width, int height);

    void run();

private:
    Application() : mWindowHandle(NULL), mWindowInstance(NULL), mStopRendering{ false }
    { }

    ~Application() { destroyWindow(); }

    void destroyWindow();

    HWND               mWindowHandle;
    HINSTANCE        mWindowInstance;
    std::atomic_bool mStopRendering;
};

}