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

#ifndef ENGINE_MAJOR_VERSION
#define ENGINE_MAJOR_VERSION 1
#endif // !ENGINE_MAJOR_VERSION

#ifndef ENGINE_MINOR_VERSION
#define ENGINE_MINOR_VERSION 0
#endif // !ENGINE_MONOR_VERSION

#ifndef ENGINE_PATCH_VERSION
#define ENGINE_PATCH_VERSION 1
#endif // !ENGINE_MONOR_VERSION

#define ENGINE_VERSION (ENGINE_MAJOR_VERSION << 16) + (ENGINE_MINOR_VERSION << 8) + ENGINE_PATCH_VERSION

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

#ifndef POLYP_WIN_CLASS_NAME
#define POLYP_WIN_CLASS_NAME "PolypWindowClass"
#endif

#ifndef POLYP_WIN_TITLE
#define POLYP_WIN_TITLE "Polypious (by Polyp &Ko)"
#endif

#ifndef POLYP_WIN_APP_NAME
#define POLYP_WIN_APP_NAME "Polyp"
#endif

namespace polyp {
namespace constants {
/// Vulkan constants
inline constexpr auto      kFenceTimeout            = 2'000'000'000ULL;

/// Default camera values
inline constexpr float     kSensitivity             = 50.f;
inline constexpr float     kZoom                    = 45.0f;
inline constexpr float     kMoveSpeed               = 1.0f;
inline constexpr glm::vec3 kCameraInitPos           = glm::vec3(0.0f,  0.0f, 3.0f);
inline constexpr glm::vec3 kCameraInitLookAt        = glm::vec3(0.0f,  0.0f, 0.0f);
inline constexpr glm::vec3 kCameraWorldUp           = glm::vec3(0.0f, -1.0f, 0.0f);

} // namespace constants
} // namespace polyp
