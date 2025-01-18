#pragma once

#ifdef WIN32
#include <Windows.h>
#endif

namespace polyp {
namespace utils {

/// Returns true if the mouse cursor is within the boundaries of the Windows screen.
bool CheckCursorPosition(HWND win)
{
    RECT rect;
    GetClientRect(win, &rect);

    POINT cursor;
    GetCursorPos(&cursor);
    ScreenToClient(win, &cursor);

    return cursor.x >= rect.left && cursor.x <= rect.right &&
           cursor.y >= rect.top && cursor.y <= rect.bottom;
}

}
}
