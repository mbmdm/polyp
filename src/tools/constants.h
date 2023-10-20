#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace polyp {
namespace constants {

inline constexpr auto kWindowTitle             = "Polypious (by Polyp &Ko)";
inline constexpr auto kWindowClassName         = "PolypWindowClass";
inline constexpr auto kInternalApplicationName = "Polyp";
inline constexpr auto kMajorVersion = 99LU;
inline constexpr auto kMinorVersion = 99LU;
inline constexpr auto kPatchVersion = 99LU;

// Vulkan constants
inline constexpr auto kFenceTimeout = 2'000'000'000ULL;


namespace camera { // Default camera values

inline constexpr float kYaw = -90.0f;
inline constexpr float kPitch = 0.0f;
inline constexpr float kSpeed = 2.5f;
inline constexpr float kSensitivity = 0.1f;
inline constexpr float kZoom = 45.0f;

} // namespace camera

namespace vk { // specific vulkan constants

inline constexpr auto kVkLibraryName = "vulkan-1.dll";

} // namespace vk

} // namespace constants
} // namespace polyp

#endif //CONSTANTS_H
