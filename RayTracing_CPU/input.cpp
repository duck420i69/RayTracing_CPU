#include "input.h"

#include "integrater/DebugRenderer.h"
#include "threading.h"

#include <algorithm>

static double mouse_x = 0.0, mouse_y = 0.0;


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    InputData* data = reinterpret_cast<InputData*>(glfwGetWindowUserPointer(window));

    int WINDOW_WIDHT;
    int WINDOW_HEIGHT;
    glfwGetWindowSize(window, &WINDOW_WIDHT, &WINDOW_HEIGHT);

    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        data->rayPaths = shootRayDebug(*data->scene, get_buffer_pos(WINDOW_WIDHT, WINDOW_HEIGHT), 10);
    }
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputData* data = reinterpret_cast<InputData*>(glfwGetWindowUserPointer(window));
    ThreadPool& threadPool = *data->threadPool;
    Scene& scene = *data->scene;
    Camera& cam = scene.camera;
    float& theta = data->rotationSpeed;
    float& speed = data->movementSpeed;

    vec3 up = { 0.0f, 1.0f, 0.0f };
    vec3 right = up.cross(cam.dir * -1.0f);
    right.normalize();
    vec3 front = up.cross(right);

    if (action != GLFW_RELEASE) {
        bool camera_change = true;
        switch (key) {
        case GLFW_KEY_Q: {
            vec3 rotation_matrix[3] = {
                { std::cos(theta), 0.0f, -std::sin(theta) },
                { 0.0f           , 1.0f,  0.0f            },
                { std::sin(theta), 0.0f,  std::cos(theta) }
            };
            cam.dir = rotation_matrix[0] * cam.dir.x() + rotation_matrix[1] * cam.dir.y() + rotation_matrix[2] * cam.dir.z();
            cam.updateCamera(cam.pos, cam.dir);
            break;
        }
        case GLFW_KEY_E: {
            vec3 rotation_matrix[3] = {
                { std::cos(-theta), 0.0f, -std::sin(-theta) },
                { 0.0f            , 1.0f,  0.0f             },
                { std::sin(-theta), 0.0f,  std::cos(-theta) }
            };
            cam.dir = rotation_matrix[0] * cam.dir.x() + rotation_matrix[1] * cam.dir.y() + rotation_matrix[2] * cam.dir.z();
            cam.updateCamera(cam.pos, cam.dir);
            break;
        }
        case GLFW_KEY_W: {
            cam.updateCamera(cam.pos + front * speed, cam.dir);
            break;
        }
        case GLFW_KEY_A: {
            cam.updateCamera(cam.pos + right * -speed, cam.dir);
            break;
        }
        case GLFW_KEY_S: {
            cam.updateCamera(cam.pos + front * -speed, cam.dir);
            break;
        }
        case GLFW_KEY_D: {
            cam.updateCamera(cam.pos + right * speed, cam.dir);
            break;
        }
        case GLFW_KEY_R: {
            cam.dir.y() += 0.02f * std::sqrt(1.0f - cam.dir.y() * cam.dir.y());
            if (cam.dir.y() > 0.999f) cam.dir.y() = 0.999f;
            float c = std::sqrt((1.0f - cam.dir.y() * cam.dir.y()) / (cam.dir.x() * cam.dir.x() + cam.dir.z() * cam.dir.z()));
            cam.dir.x() *= c;
            cam.dir.z() *= c;
            cam.updateCamera(cam.pos, cam.dir);
            break;
        }
        case GLFW_KEY_F: {
            cam.dir.y() -= 0.02f * std::sqrt(1.0f - cam.dir.y() * cam.dir.y());
            if (cam.dir.y() < -0.999f) cam.dir.y() = -0.999f;
            float c = std::sqrt((1.0f - cam.dir.y() * cam.dir.y()) / (cam.dir.x() * cam.dir.x() + cam.dir.z() * cam.dir.z()));
            cam.dir.x() *= c;
            cam.dir.z() *= c;
            cam.updateCamera(cam.pos, cam.dir);
            break;
        }
        case GLFW_KEY_TAB: {
            if (GLFW_PRESS)
                data->showFrame = ++(data->showFrame) % 3;
            break;
        }
        case GLFW_KEY_LEFT_SHIFT: {
            cam.updateCamera(cam.pos + up * speed, cam.dir);
            break;
        }
        case GLFW_KEY_LEFT_CONTROL: {
            cam.updateCamera(cam.pos + up * -speed, cam.dir);
            break;
        }
        case GLFW_KEY_COMMA: {
            speed *= 1.1f;
            break;
        }
        case GLFW_KEY_PERIOD: {
            speed /= 1.1f;
            break;
        }
        case GLFW_KEY_O: {
            scene.environment->rotate(0.0f, 0.314f);
            break;
        }
        case GLFW_KEY_P: {
            scene.environment->rotate(0.0f, -0.314f);
            break;
        }
        default:
            camera_change = false;
            break;
        }

        if (camera_change) {
            threadPool.updateCamera();
        }
    }
}


void get_mouse_pos(double* x, double* y) {
    *x = mouse_x;
    *y = mouse_y;
}

long get_buffer_pos(const int& width, const int& height) {
    return std::max(0.0, std::min(mouse_x + (height - mouse_y) * width, double(width * height - 1)));
}