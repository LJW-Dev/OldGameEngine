#pragma once

#include <GLFW/glfw3.h>

void window_keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_FocusCallback(GLFWwindow* window, int focused);
void window_closeCallback(GLFWwindow* window);