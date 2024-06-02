#ifndef CAMERA_H
#define CAMERA_H

#include "constants.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace polyp {
namespace tools {

enum class Direction
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera
{
public:
    Camera(
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
        float yaw = constants::kYaw, 
        float pitch = constants::kPitch);

    void process_keyboard(Direction direction, float deltaTime);
    void process_mouse(float xoffset, float yoffset, bool constrainPitch = true);
    void process_scroll(float yoffset);
    float zoom() const { return mZoom; };
    void zoom(float zoom) { mZoom = zoom; };
    float sensitivity() const { return mSensitivity; };
    void sensitivity(float sensitivity) { mSensitivity = sensitivity; };
    float speed() const { return mSpeed; };
    void speed(float speed) { mSpeed = speed; };
    void reset(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f));

    glm::mat4 view();

private:
    glm::vec3 mPosition;
    glm::vec3 mFront;
    glm::vec3 mUp;
    glm::vec3 mRight;
    glm::vec3 mWorldUp;

    float mYaw; // Euler Angle yaw
    float mPitch; // Euler Angle pitch

    float mSpeed; // movement speed
    float mSensitivity; // mouse sensitivity
    float mZoom;

    void update();
};

} // tools
} // polyp

#endif //CAMERA_H
