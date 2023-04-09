#ifndef PLATFORMS_H
#define PLATFORMS_H

#define NOMINMAX
#include <Windows.h>

namespace polyp {
namespace tools {

using WindowsInstance = HINSTANCE;
using WindowsHandle   = HWND;

} // utils
} // polyp

#endif
