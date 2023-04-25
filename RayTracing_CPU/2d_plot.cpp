#include "GLFW/glfw3.h"
#include "math.h"
#include <iostream>


float WINDOW_WIDHT = 1200;
float WINDOW_HEIGHT = 800;

inline void DrawPixel(std::vector<vec4>& frameBuffer, double x, double y, const vec4& color) {
    if (0.0 <= x && x < WINDOW_WIDHT && 0.0 <= y && y < WINDOW_HEIGHT)
        frameBuffer[(int)x + (int)y * WINDOW_WIDHT] = color;
}

void DrawLine(std::vector<vec4>& frameBuffer, double x1, double y1, double x2, double y2, const vec4& color) {
    if (x1 == x2) {
        if (y1 > y2) std::swap(y1, y2);
        for (; y1 <= y2; y1 = y1 + 1.0) {
            if (0.0 <= x1 && x1 < WINDOW_WIDHT && 0.0 <= y1 && y1 < WINDOW_HEIGHT)
                frameBuffer[(int)x1 + (int)y1 * WINDOW_WIDHT] = color;
        }
    }
    else {
        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
        double a = (y2 - y1) / (x2 - x1);
        double b = y1 - a * x1;
        x1 = x1 + 0.5;
        if (-1.0 < a && a < 1.0)
            while (x1 <= x2) {
                if (0.0 <= x1 && x1 < WINDOW_WIDHT && 0 <= (int)(a * x1 + b) && (int)(a * x1 + b) < WINDOW_HEIGHT)
                    frameBuffer[(int)x1 + (int)(a * x1 + b) * WINDOW_WIDHT] = color;
                x1 = x1 + 1.0;
            }
        else {
            if (y1 > y2) {
                std::swap(x1, x2);
                std::swap(y1, y2);
            }
            while (y1 <= y2) {
                if (0.0 <= (int)((y1 - b) / a) && (int)((y1 - b) / a) < WINDOW_WIDHT && 0 <= (int)y1 && (int)y1 < WINDOW_HEIGHT)
                    frameBuffer[(int)((y1 - b)/a) + (int)y1 * WINDOW_WIDHT] = color;
                y1 = y1 + 1.0;
            }
        }
    }
}

double func(double x) {
    double m = 0.2;
    double n = 2 * pi / (4 * m * m) - 1;
    return pow(cos(x), n)*(n + 1)/(2 * pi);
}

/*
int main() {
    GLFWwindow* window;

    vec2 zoom = { 3.0f, 2.0f };
    vec2 camPos = { 0.0f, 0.0f };

    double dPixelX = zoom.x / WINDOW_WIDHT * 2.0;
    double dPixelY = zoom.y / WINDOW_HEIGHT * 2.0;

    std::vector<vec4> frameBuffer;
    frameBuffer.resize(WINDOW_WIDHT * WINDOW_HEIGHT);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WINDOW_WIDHT, WINDOW_HEIGHT, "Plot 2D!", NULL, NULL);

    glfwSetWindowUserPointer(window, &zoom);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    double prev_x = 0.0;
    double prev_y = 0.0;
    double x0 = -camPos.x / dPixelX + WINDOW_WIDHT / 2;
    double y0 = -camPos.y / dPixelY + WINDOW_HEIGHT / 2;

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camPos.y += 0.01 * zoom.x;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camPos.x -= 0.01 * zoom.x;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camPos.x += 0.01 * zoom.x;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camPos.y -= 0.01 * zoom.x;
        }
        glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
            vec2* zoom = (vec2*)glfwGetWindowUserPointer(window);
            zoom->x *= pow(2.0, yoffset / 2.5);
            zoom->y *= pow(2.0, yoffset / 2.5);
            });
        
        dPixelX = zoom.x / WINDOW_WIDHT * 2.0;
        dPixelY = zoom.y / WINDOW_HEIGHT * 2.0;
        x0 = -camPos.x / dPixelX + WINDOW_WIDHT / 2;
        y0 = -camPos.y / dPixelY + WINDOW_HEIGHT / 2;

        for (int i = 0; i < WINDOW_WIDHT * WINDOW_HEIGHT; i++) {
            frameBuffer[i] = { 1.0f, 1.0f, 1.0f, 1.0f };
        }
        DrawLine(frameBuffer, 0.0, y0, 1200.0, y0, { 0.0f, 0.0f, 0.0f, 1.0f });
        DrawLine(frameBuffer, x0, 0.0, x0, 800.0, { 0.0f, 0.0f, 0.0f, 1.0f });
        for (int i = -WINDOW_WIDHT / 2; i < WINDOW_WIDHT / 2; i++) {
            double x = dPixelX * i + camPos.x;
            double y = (func(x) - camPos.y) / dPixelY;
            x = i + WINDOW_WIDHT / 2;
            y = y + WINDOW_HEIGHT / 2;
            if (i == -WINDOW_WIDHT / 2)
                DrawPixel(frameBuffer, x, y, { 1.0f, 0.0f, 0.0f, 0.0f });
            else
                DrawLine(frameBuffer, prev_x, prev_y, x, y, { 1.0f, 0.0f, 0.0f, 0.0f });
            prev_x = x;
            prev_y = y;
        }

        glDrawPixels(WINDOW_WIDHT, WINDOW_HEIGHT, GL_RGBA, GL_FLOAT, &frameBuffer.front());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
*/