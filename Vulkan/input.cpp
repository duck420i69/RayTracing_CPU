#include "input.h"
#include <glm/gtx/transform.hpp>


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));

    float theta = 0.0314f;
    float speed = 10.0f;

    glm::vec3 up = { 0.0f, 1.0f, 0.0f };

    if (action != GLFW_RELEASE) {
        bool camera_change = true;
        switch (key) {
        case GLFW_KEY_Q: {
            camera->direction = glm::rotate(theta, up) * glm::vec4(camera->direction, 1.0f);
            break;
        }
        case GLFW_KEY_E: {
            camera->direction = glm::rotate(-theta, up) * glm::vec4(camera->direction, 1.0f);
            break;
        }
        case GLFW_KEY_W: {
            camera->position = camera->position + camera->direction * speed;
            break;
        }
        case GLFW_KEY_A: {

            break;
        }
        case GLFW_KEY_S: {
            camera->position = camera->position + camera->direction * -speed;
            break;
        }
        case GLFW_KEY_D: {

            break;
        }
        case GLFW_KEY_R: {

            break;
        }
        case GLFW_KEY_F: {

            break;
        }
        case GLFW_KEY_LEFT_SHIFT: {
            camera->position = camera->position + up * speed;
            break;
        }
        case GLFW_KEY_LEFT_CONTROL: {
            camera->position = camera->position + up * -speed;
            break;
        }
        default:

            break;
        }
    }
}