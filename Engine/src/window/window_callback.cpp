#include "window_callback.h"
#include "Window.h"
#include "src/input/Input.h"

void window_keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	input_keyPressCallback(key, action, mods);
}

void window_FocusCallback(GLFWwindow* window, int focused)
{
	window_setFocus(focused);
}

void window_closeCallback(GLFWwindow* window)
{
	window_windowShouldClose();
}