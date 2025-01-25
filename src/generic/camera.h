#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace polyp {

class Camera
{
public:
    enum class Direction
    {
        None,
        Forward,
        Backward,
        Left,
        Right,
        Up,
        Down,
    };

    Camera(glm::vec3 position, glm::vec3 worldUp, glm::vec3 lookAt): mWorldUp{ worldUp }
    {
        reset(position, lookAt);
    }

    void processKeyboard(Direction direction, float deltaTime);
    void procesMouse(float xoffset, float yoffset, float deltaTime);

    float sensitivity() const { return mSensitivity; };
    void sensitivity(float sensitivity) { mSensitivity = sensitivity; };

    float speed() const { return mSpeed; };
    void speed(float speed) { mSpeed = speed; };

    void reset(glm::vec3 position, glm::vec3 lookAt);

    glm::mat4 view();

private:
    glm::vec3 mUp;
    glm::vec3 mFront;
    glm::vec3 mRight;
    glm::vec3 mWorldUp;
    glm::mat4 mCachedView;

    glm::vec3 mPosition;
    glm::vec3 mDefaultPosition;

    float mYaw   = -90.0f; // Euler Angle yaw
    float mPitch =   0.0f; // Euler Angle pitch

    float mSpeed       = 1.0; // movement speed
    float mSensitivity = 1.0; // mouse sensitivity

    bool dirtyView        = true;
    bool dirtyOrientation = true;

    void updateView();
    void updateOrientation();
};

} // polyp
