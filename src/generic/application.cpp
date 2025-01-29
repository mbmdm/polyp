#include "application.h"

#include <os_utils.h>

#ifdef WIN32
#include <windowsx.h>
#include <winuser.h>
#endif

#include <unordered_map>
#include <thread>
#include <cctype>

namespace {

using namespace polyp;

const char* kPolypWindowClassName = "PolypWindowClass";

enum class UserMessage
{
    Resize = WM_USER + 1,
    Resized,
    Quit,
    MouseClick,
    MouseMove,
    MouseWheel,
    KeyPress,
    KeyRelease
};

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
        PostMessage(hWnd, static_cast<int>(UserMessage::MouseClick), static_cast<int>(MouseButton::Left), static_cast<int>(MouseActioin::Click));
        break;
    case WM_LBUTTONUP:
        PostMessage(hWnd, static_cast<int>(UserMessage::MouseClick), static_cast<int>(MouseButton::Left), static_cast<int>(MouseActioin::Release));
        break;
    case WM_RBUTTONDOWN:
        PostMessage(hWnd, static_cast<int>(UserMessage::MouseClick), static_cast<int>(MouseButton::Right), static_cast<int>(MouseActioin::Click));
        break;
    case WM_RBUTTONUP:
        PostMessage(hWnd, static_cast<int>(UserMessage::MouseClick), static_cast<int>(MouseButton::Right), static_cast<int>(MouseActioin::Release));
        break;
    case WM_MOUSEMOVE:
        PostMessage(hWnd, static_cast<int>(UserMessage::MouseMove), wParam, lParam);
        break;
    case WM_MOUSEWHEEL:
        PostMessage(hWnd, static_cast<int>(UserMessage::MouseWheel), wParam, lParam);
        break;
    case WM_SIZE:
        PostMessage(hWnd, static_cast<int>(UserMessage::Resize), wParam, lParam);
        break;
    case WM_EXITSIZEMOVE:
        PostMessage(hWnd, static_cast<int>(UserMessage::Resized), 0, 0);
        break;
    case WM_KEYDOWN:
        if (VK_ESCAPE == wParam) {
            PostMessage(hWnd, static_cast<int>(UserMessage::Quit), 0, 0);
            break;
        }
        PostMessage(hWnd, static_cast<int>(UserMessage::KeyPress), wParam, lParam);
        break;
    case WM_KEYUP:
        PostMessage(hWnd, static_cast<int>(UserMessage::KeyRelease), wParam, lParam);
        break;
    case WM_CLOSE:
        PostMessage(hWnd, static_cast<int>(UserMessage::Quit), 0, 0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

class DPIScale
{
public:
    static void init(HWND windowHandle)
    {
        float dpi = GetDpiForWindow(windowHandle);
        scale = dpi / 96.0f;
    }

    template <typename T>
    static float convert(T val)
    {
        return static_cast<float>(val) / scale;
    }

private:
    static float scale;
};

float DPIScale::scale = 1.0f;
}

namespace polyp {

bool Application::init(const char* title, int width, int height)
{
    if (mWindowInstance) {
        destroyWindow();
    }

    mWindowInstance = GetModuleHandle(nullptr);

    WNDCLASSEX winClass = {
        sizeof(WNDCLASSEX),                  // UINT      cbSize
        CS_HREDRAW | CS_VREDRAW,             // UINT      style
        WindowProcedure,                     // WNDPROC   lpfnWndProc
        0,                                   // int       cbClsExtra
        0,                                   // int       cbWndExtra
        mWindowInstance,                     // HINSTANCE hInstance
        nullptr,                             // HICON     hIcon
        LoadCursor(nullptr, IDC_ARROW),      // HCURSOR   hCursor
        (HBRUSH)(COLOR_WINDOW + 1),          // HBRUSH    hbrBackground
        nullptr,                             // LPCSTR    lpszMenuName
        kPolypWindowClassName,               // LPCSTR    lpszClassName
        nullptr                              // HICON     hIconSm
    };

    if (!RegisterClassEx(&winClass)) {
        return false;
    }

    mWindowHandle = CreateWindow(kPolypWindowClassName, title, WS_OVERLAPPEDWINDOW, 0, 0, width, height, nullptr, nullptr, mWindowInstance, nullptr);
    if (!mWindowHandle) {
        return false;
    }

    DPIScale::init(mWindowHandle);

    WindowInitializedEventArgs args{ mWindowHandle, mWindowInstance };
    onWindowInitialized(args);

    return true;
}

void Application::run()
{
    if (!mWindowHandle || !mWindowInstance) {
        return;
    }

    ShowWindow(mWindowHandle, SW_SHOWNORMAL);
    UpdateWindow(mWindowHandle);

    MSG message;
    MovementEventArgs movement{};

    std::atomic_bool trackCursor{ false };

    auto trackCursorFunc = [](HWND win, std::atomic_bool& output, std::atomic_bool& stopToken) {
        while (!stopToken) {
            output.store(utils::CheckCursorPosition(win), std::memory_order_relaxed);
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    };
    std::thread trackCursorThread{ trackCursorFunc, mWindowHandle, std::ref(trackCursor), std::ref(mStopRendering) };
    
    while (!mStopRendering.load())
    {
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            switch (static_cast<UserMessage>(message.message))
            {
                case UserMessage::MouseClick:
                {
                    MouseClickEventArgs args {
                        static_cast<MouseButton>(message.wParam),
                        static_cast<MouseActioin>(message.lParam)
                    };
                    onMouseClick(args);
                    break;
                }
                case UserMessage::MouseMove:
                {
                    movement.mouse.x = DPIScale::convert(GET_X_LPARAM(message.lParam));
                    movement.mouse.y = DPIScale::convert(GET_Y_LPARAM(message.lParam));
                    break;
                }
                case UserMessage::MouseWheel:
                {
                    movement.mouse.wheel += GET_WHEEL_DELTA_WPARAM(message.wParam) / WHEEL_DELTA;
                    movement.mouse.x      = DPIScale::convert(GET_X_LPARAM(message.lParam));
                    movement.mouse.y      = DPIScale::convert(GET_Y_LPARAM(message.lParam));
                    break;
                }
                case UserMessage::Resize:
                {
                    WindowResizeEventArgs args
                    {
                        .mode   = static_cast<WindowResizeMode>(message.wParam),
                        .width  = HIWORD(message.lParam),
                        .height = LOWORD(message.lParam)
                    };
                    onWindowResized(args);
                    break;
                }
                case UserMessage::Resized:
                {
                    WindowResizeEventArgs args
                    {
                        .mode   = WindowResizeMode::ExitSizeMove,
                        .width  = 0,
                        .height = 0
                    };
                    onWindowResized(args);
                    break;
                }
                case UserMessage::Quit:
                {
                    mStopRendering.store(true);
                    break;
                }
                case UserMessage::KeyPress:
                {
                    if ((message.wParam >> 8) > 0)
                        break;

                    char key = static_cast<char>(message.wParam);

                    if (key == VK_SPACE)
                        movement.move.up = true;
                    else if (key == VK_CONTROL)
                        movement.move.down = true;
                    else if (key == VK_END)
                        movement.reset = true;

                    key = std::tolower(key);

                    if (key == 'w')
                        movement.move.ahead = true;
                    else if (key == 's')
                        movement.move.back = true;
                    else if (key == 'a')
                        movement.move.left = true;
                    else if (key == 'd')
                        movement.move.righ = true;

                    break;
                }
                case UserMessage::KeyRelease:
                {
                    if ((message.wParam >> 8) > 0)
                        break;

                    char key = static_cast<char>(message.wParam);

                    if (key == VK_SPACE)
                        movement.move.up = false;
                    else if (key == VK_CONTROL)
                        movement.move.down = false;
                    else if (key == VK_END)
                        movement.reset = false;

                    key = std::tolower(key);

                    if (key == 'w')
                        movement.move.ahead = false;
                    else if (key == 's')
                        movement.move.back = false;
                    else if (key == 'a')
                        movement.move.left = false;
                    else if (key == 'd')
                        movement.move.righ = false;

                    break;
                }
            }

            TranslateMessage(&message);
            DispatchMessage(&message);

        }

        if (!trackCursor.load(std::memory_order_relaxed))
        {
            MouseClickEventArgs args{ MouseButton::Left, MouseActioin::Release };
            onMouseClick(args);
        }

        onMovement(movement);
        onRender();
    }

    onShutdown();

    trackCursorThread.join();
}

void Application::destroyWindow()
{
    if (mWindowHandle) {
        DestroyWindow(mWindowHandle);
        mWindowHandle = NULL;
    }
    if (mWindowInstance) {
        UnregisterClass(kPolypWindowClassName, mWindowInstance);
        mWindowInstance = NULL;
    }
}

}
