#if 0

#include "GLFW/glfw3.h"
#include "integrater/Renderer.h"

#include "rtmath.h"

#include <iostream>



float WINDOW_WIDHT = 1200;
float WINDOW_HEIGHT = 800;


double func(double x) {
    double m = 0.2;
    double n = 2 * pi / (4 * m * m) - 1;
    return pow(cos(x), n)*(n + 1)/(2 * pi);
}


int main() {
    GLFWwindow* window;

    Camera camera = Camera(
        { 0.0f, 0.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, -1.0f, 1.0f },
        { WINDOW_WIDHT, WINDOW_HEIGHT },
        45.0f);

    std::vector<vec4> lines;
    lines.push_back({ 1.5f, 1.0f, 0.0f, 1.0f });
    lines.push_back({ -1.5f, 1.0f, 0.0f, 1.0f });
    lines.push_back({ -1.5f, -1.0f, 0.0f, 1.0f });
    lines.push_back({ 1.5f, -1.0f, 0.0f, 1.0f });
    lines.push_back({ 3.0f, 2.0f, -1.0f, 1.0f });
    lines.push_back({ 1.5f, 1.0f, -1.0f, 1.0f });
    lines.push_back({ -1.5f, 1.0f, -1.0f, 1.0f });
    lines.push_back({ -1.5f, -1.0f, -1.0f, 1.0f });
    lines.push_back({ 1.5f, -1.0f, -1.0f, 1.0f });
    lines.push_back({ 1.5f, 1.0f, -1.0f, 1.0f });

    Buffer frameBuffer((int)WINDOW_WIDHT, (int)WINDOW_HEIGHT);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WINDOW_WIDHT, WINDOW_HEIGHT, "Plot 2D!", NULL, NULL);

    glfwSetWindowUserPointer(window, &camera.pos);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);


    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera.pos.y() += 0.05f * camera.pos.z();
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera.pos.x -= 0.05f * camera.pos.z();
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera.pos.x += 0.05f * camera.pos.z();
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera.pos.y() -= 0.05f * camera.pos.z();
        }
        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            vec4* pos = (vec4*)glfwGetWindowUserPointer(window);
            pos->v(0) *= pow(2.0, yoffset / 2.5);
            pos->v(1) *= pow(2.0, yoffset / 2.5);
            });

        for (int i = 0; i < frameBuffer.w * frameBuffer.h; i++) {
            frameBuffer.frameBuffer[i] = { 0.2f, 0.2f, 0.2f, 1.0f };
        }

        for (int i = 0; i < lines.size() - 1; i++) {
            drawLine(frameBuffer, camera, lines[i], lines[i + 1], { 1.0f, 1.0f, 1.0f, 1.0f });
        }

        glDrawPixels(WINDOW_WIDHT, WINDOW_HEIGHT, GL_RGBA, GL_FLOAT, &frameBuffer.frameBuffer.front());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
#endif