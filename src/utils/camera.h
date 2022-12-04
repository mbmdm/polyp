#ifndef CAMERA_H
#define CAMERA_H

#include "constants.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace utils 
{
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
            float yaw = constants::camera::kYaw, 
            float pitch = constants::camera::kPitch);

        void process_keyboard(Direction direction, float deltaTime);
        void process_mouse(float xoffset, float yoffset, GLboolean constrainPitch = true);
        void process_scroll(float yoffset);
        float get_zoom() { return mZoom; };

        glm::mat4 get_view_mtx();

    private:
        glm::vec3 mPosition;
        glm::vec3 mFront;
        glm::vec3 mUp;
        glm::vec3 mRight;
        glm::vec3 mWorldUp;

        // Euler Angles
        float mYaw;
        float mPitch;

        float mMovementSpeed;
        float mMouseSensitivity;
        float mZoom;

        void updateCameraVectors();
    };
}
#endif
