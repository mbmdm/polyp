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

    Camera(glm::vec3 position) :
        mPosition{ position }, mYaw{ -90.0f }, mPitch{ 0.0 }
    {
        dirtyView        = true;
        dirtyOrientation = true;
    }

    Camera(glm::vec3 position, float yaw, float pitch) : Camera(position)
    {
        mYaw   = yaw;
        mPitch = pitch;
    }

    void processKeyboard(Direction direction, float deltaTime);
    void procesMouse(float xoffset, float yoffset, float deltaTime);

    float sensitivity() const { return mSensitivity; };
    void sensitivity(float sensitivity) { mSensitivity = sensitivity; };

    float speed() const { return mSpeed; };
    void speed(float speed) { mSpeed = speed; };

    glm::vec3 position() const { return mPosition; }
    void position(glm::vec3 pos) { mPosition = pos; }

    void reset(glm::vec3 position, float yaw, float pitch);

    glm::mat4 view();

private:
    glm::vec3 mUp         = glm::vec3(0.0f, 1.0f,  0.0f);
    glm::vec3 mFront      = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 mRight      = glm::vec3(1.0f, 0.0f,  0.0f);
    glm::vec3 mWorldUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    glm::mat4 mCachedView = glm::mat4(1.0f);

    glm::vec3 mPosition;

    float mYaw;   // Euler Angle yaw
    float mPitch; // Euler Angle pitch

    float mSpeed       = 1.0; // movement speed
    float mSensitivity = 1.0; // mouse sensitivity

    bool dirtyView        = true;
    bool dirtyOrientation = true;

    void updateView();
    void updateOrientation();
};

} // polyp
