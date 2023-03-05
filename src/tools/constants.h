#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace polyp {
namespace tools {
namespace constants {

namespace camera {

// Default camera values
inline constexpr float kYaw = -90.0f;
inline constexpr float kPitch = 0.0f;
inline constexpr float kSpeed = 2.5f;
inline constexpr float kSensitivity = 0.1f;
inline constexpr float kZoom = 45.0f;

} // namespace camera

namespace window {

    // Default window surface values
    inline constexpr const char* kWindowClassName = "PolypWindow";

} // namespace window

} // constants
} // utils
} // polyp

#endif //CONSTANTS_H
