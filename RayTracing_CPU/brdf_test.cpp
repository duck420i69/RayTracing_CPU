#include "GLFW/glfw3.h"
#include "Camera.h"
#include "input.h"
#include "loader.h"
#include <execution>
#include <iostream>
#include <iomanip>


/*
vec3 aces_approx(vec3 v)
{
    v *= 0.6f;
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    vec3 result;
    for (int i = 0; i < 3; i++) result(i) = std::clamp((v(i) * (a * v(i) + b) / (v(i) * (c * v(i) + d)) + e), 0.0f, 1.0f);
    return result;
}



int main() {
    GLFWwindow* window;

    float WINDOW_WIDHT = 1200;
    float WINDOW_HEIGHT = 800;

    std::vector<vec4> frameBuffer;
    frameBuffer.resize(WINDOW_WIDHT * WINDOW_HEIGHT);

    vec4 direction = { 1.0f, 0.0f, 0.0f, 1.0f };
    direction.normalize();

    Camera cam({ 0.0f, 0.0f, 0.0f, 1.0f }, direction, { WINDOW_WIDHT, WINDOW_HEIGHT }, 45);

    bool mouse_click = false;
    bool hold_tab = false;
    bool up_down = true;

    const float theta = 1.0f / 200.0f * pi;
    const float speed = 0.01f;

    vec4 up = { 0.0f, 1.0f, 0.0f, 1.0f };
    vec4 right = { 0.0f, 0.0f, 1.0f, 1.0f };
    vec4 front = { 1.0f, 0.0f, 0.0f, 1.0f };

    std::vector<vec4> points;
    std::vector<vec4> xzplane;
    int error = 0;
    Plastic material;
    material.diffuse_color = { 1.0f, 0.0f, 0.0f, 0.0f };
    material.specular_color = { 1.0f, 0.0f, 0.0f, 0.0f };
    material.specular_exp = 120;

    for (float i = -5.0f; i <= 5.0f; i += 0.01f) {
        xzplane.push_back({ i, 0.0f, 0.0f, 1.0f });
        xzplane.push_back({ 0, 5.0f + i, 0.0f, 1.0f });
        xzplane.push_back({ 0.0f, 0.0f, i, 1.0f });
    }

    Scene scene;
    //scene.environment = loadEnvHDR("skybox/kloppenheim_05_puresky_2k.hdr");
    scene.environment = loadEnvHDR("skybox/syferfontein_6d_clear_puresky_4k.hdr");

    points.clear();
    for (long i = 0; i < 100000; i++) {
        auto sample = scene.environment->generateSample(Packet());
        vec4 point = sample.sample;
        point = point * (1.0f / sample.inv_pdf);
        points.push_back(point);
    }

    direction = { 1.0f, -1.0f, 1.0f, 1.0f };
    direction.normalize();
    Ray ray(direction, direction);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WINDOW_WIDHT, WINDOW_HEIGHT, "BRDF Testing", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    Eigen::Matrix3f cameraBase({
        { cam.right.v(0), -cam.down.v(0), cam.dir.v(0) },
        { cam.right.v(1), -cam.down.v(1), cam.dir.v(1) },
        { cam.right.v(2), -cam.down.v(2), cam.dir.v(2) } });

    cameraBase = cameraBase.inverse();

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        bool camera_change = false;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            vec4 rotation_matrix[3] = {
                {std::cos(theta), 0.0f, -std::sin(theta), 0.0f},
                {0.0f, 1.0f, 0.0f, 0.0f},
                {std::sin(theta), 0.0f, std::cos(theta), 0.0f}
            };
            cam.dir = rotation_matrix[0] * cam.dir.v(0) + rotation_matrix[1] * cam.dir.v(1) + rotation_matrix[2] * cam.dir.v(2);
            cam.updateCamera(cam.pos, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            vec4 rotation_matrix[3] = {
                {std::cos(-theta), 0.0f, -std::sin(-theta), 0.0f},
                {0.0f, 1.0f, 0.0f, 0.0f},
                {std::sin(-theta), 0.0f, std::cos(-theta), 0.0f}
            };
            cam.dir = rotation_matrix[0] * cam.dir.v(0) + rotation_matrix[1] * cam.dir.v(1) + rotation_matrix[2] * cam.dir.v(2);
            cam.updateCamera(cam.pos, cam.dir);
            camera_change = true;
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            cam.dir.v(1) += 0.02f * std::sqrt(1.0f - cam.dir.v(1) * cam.dir.v(1));
            if (cam.dir.v(1) > 0.999f) cam.dir.v(1) = 0.999f;
            float c = std::sqrt((1.0f - cam.dir.v(1) * cam.dir.v(1)) / (cam.dir.v(0) * cam.dir.v(0) + cam.dir.v(2) * cam.dir.v(2)));
            cam.dir.v(0) *= c;
            cam.dir.v(2) *= c;
            cam.updateCamera(cam.pos, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            cam.dir.v(1) -= 0.02f * std::sqrt(1.0f - cam.dir.v(1) * cam.dir.v(1));
            if (cam.dir.v(1) < -0.999f) cam.dir.v(1) = -0.999f;
            float c = std::sqrt((1.0f - cam.dir.v(1) * cam.dir.v(1)) / (cam.dir.v(0) * cam.dir.v(0) + cam.dir.v(2) * cam.dir.v(2)));
            cam.dir.v(0) *= c;
            cam.dir.v(2) *= c;
            cam.updateCamera(cam.pos, cam.dir);
            camera_change = true;
        }


        right = up.cross(cam.dir * -1.0f);
        right.normalize();
        front = up.cross(right);


        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + front * speed, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + right * -speed, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + right * speed, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + front * -speed, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + up * speed, cam.dir);
            camera_change = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            cam.updateCamera(cam.pos + up * -speed, cam.dir);
            camera_change = true;
        }

        if (camera_change) {
            cameraBase = Eigen::Matrix3f({
                { cam.right.v(0), -cam.down.v(0), cam.dir.v(0) },
                { cam.right.v(1), -cam.down.v(1), cam.dir.v(1) },
                { cam.right.v(2), -cam.down.v(2), cam.dir.v(2) } });
            cameraBase = cameraBase.inverse();
        }

        if (up_down) {
            direction.v(1) += 0.002f * std::sqrt(1.0f - direction.v(1) * direction.v(1));
            if (direction.v(1) >= -0.001f) {
                direction.v(1) = -0.001f;
                up_down = false;
            }
            float c = std::sqrt((1.0f - direction.v(1) * direction.v(1)) / (direction.v(0) * direction.v(0) + direction.v(2) * direction.v(2)));
            direction.v(0) *= c;
            direction.v(2) *= c;
            ray = Ray(direction, direction);
        }
        else {
            direction.v(1) -= 0.002f * std::sqrt(1.0f - direction.v(1) * direction.v(1));
            if (direction.v(1) <= -0.999f) {
                direction.v(1) = -0.999f;
                up_down = true;
            }
            float c = std::sqrt((1.0f - direction.v(1) * direction.v(1)) / (direction.v(0) * direction.v(0) + direction.v(2) * direction.v(2)));
            direction.v(0) *= c;
            direction.v(2) *= c;
            ray = Ray(direction, direction);
        }

        //points.clear();
        //for (double i = 0.0; i < 2.0 * pi; i += pi / 100.0) {
        //    for (double j = 0.0; j <= pi / 2; j += pi / 200.0) {
        //        vec4 wo = { cosf(i) * sinf(j), cosf(j), sinf(i) * sinf(j), 1.0f };
        //        Packet packet;
        //        SampleResult sample;
        //        sample.sample = wo;
        //        packet.normal = { 0.0f, 1.0f, 0.0f, 1.0f };
        //        material.interact(ray, sample, packet);
        //        wo = wo * packet.color.v(0);
        //        points.push_back(wo);
        //    }
        //}

        atomic current = 0l;
        std::for_each(std::execution::par, frameBuffer.begin(), frameBuffer.end(), [&](vec4& dummy) {
            long i = current;
            Packet packet;
            vec4 current_ray = cam.topleft;
            current_ray = current_ray + cam.down * (cam.resolution.y - (i / (int)cam.resolution.x)) + cam.right * (i % (int)cam.resolution.x);
            current_ray.normalize();
            scene.environment->interact(Ray({ 0,0,0,0 }, current_ray), packet);

            frameBuffer[i].v = aces_approx(packet.color.v);
            frameBuffer[i].w = 1.0f;
            current++;
            });

        std::for_each(std::execution::par, points.begin(), points.end(), [&](vec4& point) {
            vec3 v = cameraBase * (point.v - cam.pos.v);
            vec3 floor = { point.v(0), 0.0f, point.v(2) };
            floor = cameraBase * (floor - cam.pos.v);
            if (v.z() > 0.0001f) {
                v.x() /= v.z();
                v.y() /= v.z();

                v.x() += width / 2;
                v.y() += height / 2;

                if (0 <= v.x() && v.x() < width && 0 <= v.y() && v.y() < height) {
                    frameBuffer[(int)v.x() + (int)v.y() * width] = { 1.0f, v.z()/ 3.0f, v.z()/ 3.0f, 1.0f};
                }
            }
            if (floor.z() > 0.0001f) {
                floor.x() /= floor.z();
                floor.y() /= floor.z();

                floor.x() += width / 2;
                floor.y() += height / 2;

                if (0 <= floor.x() && floor.x() < width && 0 <= floor.y() && floor.y() < height) {
                    frameBuffer[(int)floor.x() + (int)floor.y() * width] = { 0.0, 0.0, 1.0, 1.0f };
                }
            }
            });

        std::for_each(std::execution::par, xzplane.begin(), xzplane.end(), [&](vec4& point) {
            vec3 v = cameraBase * (point.v - cam.pos.v);
            vec3 floor = { point.v(0), 0.0f, point.v(2) };
            floor = cameraBase * (floor - cam.pos.v);
            if (v.z() > 0.0001f) {
                v.x() /= v.z();
                v.y() /= v.z();

                v.x() += width / 2;
                v.y() += height / 2;

                if (0 <= v.x() && v.x() < width && 0 <= v.y() && v.y() < height) {
                    frameBuffer[(int)v.x() + (int)v.y() * width] = { 1.0, 1.0, 1.0, 1.0f };
                }
            }
            if (floor.z() > 0.0001f) {
                floor.x() /= floor.z();
                floor.y() /= floor.z();

                floor.x() += width / 2;
                floor.y() += height / 2;

                if (0 <= floor.x() && floor.x() < width && 0 <= floor.y() && floor.y() < height) {
                    frameBuffer[(int)floor.x() + (int)floor.y() * width] = { 1.0, 1.0, 1.0, 1.0f };
                }
            }
            });



        glDrawPixels(width, height, GL_RGBA, GL_FLOAT, &frameBuffer.front());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
*/
