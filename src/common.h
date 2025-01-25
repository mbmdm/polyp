#pragma once

#include <generic/logs.h>
#include <glm/glm.hpp>

#include <chrono>
#include <array>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
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
#define ENGINE_VK_VERSION VK_API_VERSION_1_3
#endif // !ENGINE_VK_VERSION

#ifndef VMA_VULKAN_VERSION
#define VMA_VULKAN_VERSION 1003000
#endif // !VMA_VULKAN_VERSION

#ifndef VMA_STATIC_VULKAN_FUNCTIONS
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#endif // !VMA_STATIC_VULKAN_FUNCTIONS

#ifndef VMA_DYNAMIC_VULKAN_FUNCTIONS
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif // !VMA_DYNAMIC_VULKAN_FUNCTIONS

namespace polyp {
namespace constants {

inline constexpr auto      kWindowTitle             = "Polypious (by Polyp &Ko)";
inline constexpr auto      kWindowClassName         = "PolypWindowClass";
inline constexpr auto      kInternalApplicationName = "Polyp";
inline constexpr auto      kMajorVersion            = 99LU;
inline constexpr auto      kMinorVersion            = 99LU;
inline constexpr auto      kPatchVersion            = 99LU;

/// Vulkan constants
inline constexpr auto      kFenceTimeout            = 2'000'000'000ULL;

/// Default camera values
inline constexpr float     kSensitivity             = 50.f;
inline constexpr float     kZoom                    = 45.0f;
inline constexpr float     kMoveSpeed               = 1.0f;
inline constexpr glm::vec3 kCameraInitPos           = glm::vec3(0.0f,  0.0f, 3.0f);
inline constexpr glm::vec3 kCameraInitLookAt        = glm::vec3(0.0f,  0.0f, 0.0f);
inline constexpr glm::vec3 kCameraWorldUp           = glm::vec3(0.0f, -1.0f, 0.0f);

/// specific vulkan constants
inline constexpr auto      kVkLibraryName           = "vulkan-1.dll";

} // namespace constants
} // namespace polyp
