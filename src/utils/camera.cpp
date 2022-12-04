#include "camera.h"

#include <stdexcept>

namespace utils 
{    
    Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) :
        mPosition(position),
        mWorldUp(glm::normalize(up)),
        mYaw(yaw),
        mPitch(pitch),
        mSpeed(constants::camera::kSpeed), 
        mSensitivity(constants::camera::kSensitivity), 
        mZoom(constants::camera::kZoom)
    {
        update();
    }

    void Camera::reset(glm::vec3 position)
    {
        mPosition = position;
        mWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        mSensitivity = constants::camera::kSensitivity;
        mSpeed = constants::camera::kSpeed;
        update();
    }

    glm::mat4 Camera::view()
    {
        return glm::lookAt(mPosition, mPosition + mFront, mUp);
    }

    void Camera::process_keyboard(Direction direction, float deltaTime)
    {
        float velocity = mSpeed * deltaTime;
        switch (direction)
        {
        case utils::Direction::FORWARD:
            mPosition += mFront * velocity;
            break;
        case utils::Direction::BACKWARD:
            mPosition -= mFront * velocity;
            break;
        case utils::Direction::LEFT:
            mPosition -= mRight * velocity;
            break;
        case utils::Direction::RIGHT:
            mPosition += mRight * velocity;
            break;
        case utils::Direction::UP:
            mPosition += mUp * velocity;
            break;
        case utils::Direction::DOWN:
            mPosition -= mUp * velocity;
            break;
        default:
            break;
        }
    }

    void Camera::process_mouse(float xoffset, float yoffset, GLboolean constrainPitch)
    {
        xoffset *= mSensitivity;
        yoffset *= mSensitivity;

        mYaw += xoffset;
        mPitch += yoffset;

        if (constrainPitch)
        {
            if (mPitch > 89.0f)
                mPitch = 89.0f;
            else if (mPitch < -89.0f)
                mPitch = -89.0f;
        }
        update();
    }

    void Camera::process_scroll(float yoffset)
    {
        mZoom -= (float)yoffset;
        if (mZoom < 1.0f)
            mZoom = 1.0f;
        else if (mZoom > 45.0f)
            mZoom = 45.0f;
    }

    void Camera::update()
    {
        glm::vec3 front;

        front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
        front.y = sin(glm::radians(mPitch));
        front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));

        mFront = glm::normalize(front);
        mRight = glm::normalize(glm::cross(mFront, mWorldUp));
        mUp    = glm::normalize(glm::cross(mRight, mFront));
    }
}
