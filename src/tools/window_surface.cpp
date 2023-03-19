#include "window_surface.h"
#include "constants.h"

#include <assert.h> //TODO: remove

namespace {

enum UserMessage {
    kResize = WM_USER + 1, kQuit, kMouseClick, kMouseMove, kMouseWheel
};

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_LBUTTONDOWN:
        PostMessage(hWnd, kResize, 0, 1);
        break;
    case WM_LBUTTONUP:
        PostMessage(hWnd, kMouseClick, 0, 0);
        break;
    case WM_RBUTTONDOWN:
        PostMessage(hWnd, kMouseClick, 1, 1);
        break;
    case WM_RBUTTONUP:
        PostMessage(hWnd, kMouseClick, 1, 0);
        break;
    case WM_MOUSEMOVE:
        PostMessage(hWnd, kMouseMove, LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_MOUSEWHEEL:
        PostMessage(hWnd, kMouseWheel, HIWORD(wParam), 0);
        break;
    case WM_SIZE:
    case WM_EXITSIZEMOVE:
        PostMessage(hWnd, kResize, wParam, lParam);
        break;
    case WM_KEYDOWN:
        if (VK_ESCAPE == wParam) {
            PostMessage(hWnd, kQuit, wParam, lParam);
        }
        break;
    case WM_CLOSE:
        PostMessage(hWnd, kQuit, wParam, lParam);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

} // anonymous namespace

namespace polyp {
namespace tools {

WindowSurface::WindowSurface(const char* title,
                             int x, int y, int width, int height, 
                             IRenderer::Ptr renderer) :
    mWindowHandle{},
    mRenderer{ renderer },
    mInitialized{ false }
{
    mWindowHandle.inst = GetModuleHandle(nullptr);

    WNDCLASSEX winClass = {
        sizeof(WNDCLASSEX),                  // UINT      cbSize
        CS_HREDRAW | CS_VREDRAW,             // UINT      style
        WindowProcedure,                     // WNDPROC   lpfnWndProc
        0,                                   // int       cbClsExtra
        0,                                   // int       cbWndExtra
        mWindowHandle.inst,                  // HINSTANCE hInstance
        nullptr,                             // HICON     hIcon
        LoadCursor(nullptr, IDC_ARROW),      // HCURSOR   hCursor
        (HBRUSH)(COLOR_WINDOW + 1),          // HBRUSH    hbrBackground
        nullptr,                             // LPCSTR    lpszMenuName
        constants::window::kWindowClassName, // LPCSTR    lpszClassName
        nullptr                              // HICON     hIconSm
    };

    if (!RegisterClassEx(&winClass)) {
        return;
    }

    mWindowHandle.hwnd = CreateWindow(constants::window::kWindowClassName, 
        title, WS_OVERLAPPEDWINDOW, x, y, width, height, nullptr, nullptr, mWindowHandle.inst, nullptr);
    if (!mWindowHandle.hwnd) {
        return;
    }

    mInitialized = true;
}

WindowSurface::~WindowSurface() {
    if (mWindowHandle.hwnd) {
        DestroyWindow(mWindowHandle.hwnd);
    }
    if (mWindowHandle.inst) {
        UnregisterClass(constants::window::kWindowClassName, mWindowHandle.inst);
    }
}

void WindowSurface::run() {

    ShowWindow(mWindowHandle.hwnd, SW_SHOWNORMAL);
    UpdateWindow(mWindowHandle.hwnd);
    assert(!"stub... to implement");
}

} // namespace tools
} // namespace polyp
