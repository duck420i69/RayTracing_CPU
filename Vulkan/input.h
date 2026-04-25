#pragma once

#include <GLFW/glfw3.h>
#include "VulkanObject.h"


// void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// long get_buffer_pos(const int& width, const int& height);