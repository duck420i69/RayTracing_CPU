#include "input.h"
#include <algorithm>
#include <iostream>

static double mouse_x = 0.0, mouse_y = 0.0;
static bool click = false;
static bool old_click = false;


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    click = false;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && !old_click) {
            click = true;
            old_click = true;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            std::cout << mouse_x << " " << mouse_y << "\n";
        }
        else if (old_click && action != GLFW_PRESS) old_click = false;
    }
}

void get_mouse_pos(double* x, double* y) {
    *x = mouse_x;
    *y = mouse_y;
}

bool mouse_click() { return click; }

long get_buffer_pos(const int& width, const int& height) {
    return std::max(0.0, std::min(mouse_x + (height - mouse_y) * width, double(width * height - 1)));
}