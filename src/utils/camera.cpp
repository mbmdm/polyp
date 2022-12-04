#include "camera.h"

#include <stdexcept>

namespace utils 
{    
    Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : 
        mFront(glm::vec3(0.0f, 0.0f, -1.0f)), 
        mMovementSpeed(constants::camera::kSpeed), 
        mMouseSensitivity(constants::camera::kSensitivity), 
        mZoom(constants::camera::kZoom)
    {
        mPosition = position;
        mWorldUp = up;
        mYaw = yaw;
        mPitch = pitch;
        updateCameraVectors();
    }


    glm::mat4 Camera::get_view_mtx()
    {
        return glm::lookAt(mPosition, mPosition + mFront, mUp);
    }

    void Camera::process_keyboard(Direction direction, float deltaTime)
    {
        float velocity = mMovementSpeed * deltaTime;
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
        xoffset *= mMouseSensitivity;
        yoffset *= mMouseSensitivity;

        mYaw += xoffset;
        mPitch += yoffset;

        if (constrainPitch)
        {
            if (mPitch > 89.0f)
                mPitch = 89.0f;
            else if (mPitch < -89.0f)
                mPitch = -89.0f;
        }

        updateCameraVectors();
    }

    void Camera::process_scroll(float yoffset)
    {
        mZoom -= (float)yoffset;
        if (mZoom < 1.0f)
            mZoom = 1.0f;
        else if (mZoom > 45.0f)
            mZoom = 45.0f;
    }

    void Camera::updateCameraVectors()
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