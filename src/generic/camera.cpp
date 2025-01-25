#include "camera.h"

namespace polyp {

void Camera::reset(glm::vec3 position, glm::vec3 lookAt)
{
    mPosition = position;

    glm::vec3 direction = glm::normalize(lookAt - position);

    mPitch = glm::degrees(asin(direction.y));
    if (mPitch > 89.0f)
        mPitch = 89.0f;
    else if (mPitch < -89.0f)
        mPitch = -89.0f;
    mYaw = glm::degrees(std::atan(direction.z / direction.x));

    dirtyOrientation = true;
    dirtyView        = true;
}

glm::mat4 Camera::view()
{
    if (dirtyView)
        updateView();

    return mCachedView;
}

void Camera::processKeyboard(Direction direction, float deltaTime)
{
    const auto movementSpeed = mSpeed * deltaTime;

    switch (direction)
    {
        case Direction::Forward:
            mPosition += mFront * movementSpeed;
            break;
        case Direction::Backward:
            mPosition -= mFront * movementSpeed;
            break;
        case Direction::Left:
            mPosition -= mRight * movementSpeed;
            break;
        case Direction::Right:
            mPosition += mRight * movementSpeed;
            break;
        case Direction::Up:
            mPosition -= mUp * movementSpeed;
            break;
        case Direction::Down:
            mPosition += mUp * movementSpeed;
            break;
        default:
            break;
    }

    dirtyView = true;
}

void Camera::procesMouse(float xoffset, float yoffset, float deltaTime)
{
    xoffset *= mSensitivity * deltaTime;
    yoffset *= mSensitivity * deltaTime;

    if (mWorldUp.y < 0)
    {
        xoffset *= -1;
        yoffset *= -1;
    }

    mYaw   += xoffset;
    mPitch += yoffset;

    if (mPitch > 89.0f)
        mPitch = 89.0f;
    else if (mPitch < -89.0f)
        mPitch = -89.0f;

    dirtyView        = true;
    dirtyOrientation = true;
}

void Camera::updateView()
{
    if (dirtyOrientation)
        updateOrientation();

    mCachedView = glm::lookAt(mPosition, mPosition + mFront, mUp);

    dirtyView = false;
}

void Camera::updateOrientation()
{
    glm::vec3 direction;

    direction.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    direction.y = sin(glm::radians(mPitch));
    direction.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));

    mFront = glm::normalize(direction);
    mRight = glm::normalize(glm::cross(mFront, mWorldUp));
    mUp    = glm::normalize(glm::cross(mRight, mFront));

    dirtyOrientation = false;
}

} // polyp
