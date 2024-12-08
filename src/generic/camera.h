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

    Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch, float speed, float sensitivity) :
        mPosition{ position },
        mWorldUp{ worldUp },
        mYaw{ yaw },
        mPitch{ pitch },
        mSpeed{ speed },
        mSensitivity{ sensitivity },
        mFront{ glm::vec3(0.0f, 0.0f, -1.0f) },
        mRight{ glm::vec3(1.0f) },
        mUp{ glm::vec3(1.0f) },
        mCachedView{ glm::mat4(1.0f) }
    {
        dirtyView    = true;
        dirtyOrientation = true;
    }

    void processKeyboard(Direction direction, float deltaTime);
    void procesMouse(float xoffset, float yoffset);

    float sensitivity() const { return mSensitivity; };
    void sensitivity(float sensitivity) { mSensitivity = sensitivity; };

    float speed() const { return mSpeed; };
    void speed(float speed) { mSpeed = speed; };

    glm::vec3 position() const { return mPosition; }
    void position(glm::vec3 pos) { mPosition = pos; }

    void  reset(glm::vec3 position, float yaw, float pitch);

    glm::mat4 view();

private:
    glm::vec3 mPosition;
    glm::vec3 mFront;
    glm::vec3 mUp;
    glm::vec3 mRight;
    glm::vec3 mWorldUp;
    glm::mat4 mCachedView;

    float mYaw;   // Euler Angle yaw
    float mPitch; // Euler Angle pitch

    float mSpeed;       // movement speed
    float mSensitivity; // mouse sensitivity

    bool dirtyView;
    bool dirtyOrientation;

    void updateView();
    void updateOrientation();
};

} // polyp
