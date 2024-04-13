#ifndef PLATFORMS_H
#define PLATFORMS_H

#ifdef WIN32
#include <Windows.h>
#endif

namespace polyp {

#ifdef  WIN32
using WindowInstance = HINSTANCE;
using WindowHandle   = HWND;
#else //  WIN32
using WindowInstance = std::nullptr_t;
using WindowHandle   = std::nullptr_t;
#endif

} // polyp

#endif
