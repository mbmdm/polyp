#ifndef COMMON_H
#define COMMON_H

#include <logs.h>

#include <chrono>
#include <array>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <tuple>
#include <iterator>
#include <type_traits>
#include <thread>

#define CHECKRET(expr)                             \
{                                                  \
    auto temp = expr;                              \
    if (temp != VK_SUCCESS) {                      \
    printf("Expression %s\n", #expr);              \
    printf("Failed with result = %lu\n", temp);    \
    exit(1);}                                      \
}

#ifndef ENGINE_MAJOR_VERSION
#define ENGINE_MAJOR_VERSION 1
#endif // !ENGINE_MAJOR_VERSION

#ifndef ENGINE_MINOR_VERSION
#define ENGINE_MINOR_VERSION 1
#endif // !ENGINE_MONOR_VERSION

#ifndef ENGINE_NAME
#define ENGINE_NAME "POLYP"
#endif // !ENGINE_MONOR_VERSION

#ifndef ENGINE_VK_VERSION
#define ENGINE_VK_VERSION VK_API_VERSION_1_2
#endif // !ENGINE_VK_VERSION

namespace polyp {
namespace constants {

inline constexpr auto kWindowTitle             = "Polypious (by Polyp &Ko)";
inline constexpr auto kWindowClassName         = "PolypWindowClass";
inline constexpr auto kInternalApplicationName = "Polyp";
inline constexpr auto kMajorVersion            = 99LU;
inline constexpr auto kMinorVersion            = 99LU;
inline constexpr auto kPatchVersion            = 99LU;

/// Vulkan constants
inline constexpr auto kFenceTimeout            = 2'000'000'000ULL;
                                               
/// Default camera values                       
inline constexpr float kYaw                    = -90.0f;
inline constexpr float kPitch                  = 0.0f;
inline constexpr float kSpeed                  = 2.5f;
inline constexpr float kSensitivity            = 0.1f;
inline constexpr float kZoom                   = 45.0f;

/// specific vulkan constants
inline constexpr auto kVkLibraryName           = "vulkan-1.dll";

} // namespace constants
} // namespace polyp

#endif // COMMON_H
