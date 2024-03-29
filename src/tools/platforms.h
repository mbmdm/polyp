#ifndef PLATFORMS_H
#define PLATFORMS_H

#define NOMINMAX
#include <Windows.h>

namespace polyp {
namespace tools {

using WindowInstance = HINSTANCE;
using WindowHandle   = HWND;

} // utils
} // polyp

#endif
