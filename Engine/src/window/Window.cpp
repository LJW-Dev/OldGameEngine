

#include "src/window/Window.h"
#include "src/dvar/dvar_list.h"
#include "src/input/Input.h"


#include <GL/glew.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

struct s_windowInfo
{
    GLFWwindow* mainWindow;
    bool isFocused;
    bool shouldClose;
};

s_windowInfo windowInfo;

void window_windowShouldClose()
{
    windowInfo.shouldClose = true;
}

void window_setFocus(bool focused)
{
    windowInfo.isFocused = focused;
}

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

void initCallbacks()
{
    glfwSetKeyCallback(windowInfo.mainWindow, window_keyPressCallback);
    glfwSetWindowFocusCallback(windowInfo.mainWindow, window_FocusCallback);
    glfwSetWindowCloseCallback(windowInfo.mainWindow, window_closeCallback);
}

bool initWindow()
{
    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
    {
        printf("Failed to initialize GLFW\n");
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

    // Open a window and create its OpenGL context
    windowInfo.mainWindow = glfwCreateWindow(WINDOW_X, WINDOW_Y, "Engine", NULL, NULL);
    if (windowInfo.mainWindow == NULL)
    {
        printf("Failed to open GLFW window.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(windowInfo.mainWindow); // Initialize GLEW

    if (glewInit() != GLEW_OK) 
    {
        printf("Failed to initialize GLEW.\n");
        return false;
    }

    windowInfo.isFocused = true;

    initCallbacks();

    return true;
}

void swapBuffersAndPoll()
{
    // Swap buffers
    glfwSwapBuffers(windowInfo.mainWindow);
    glfwPollEvents();
}

bool shouldExitWindow()
{
    return windowInfo.shouldClose;
}

void getMousePos(double* xpos, double* ypos)
{
    glfwGetCursorPos(windowInfo.mainWindow, xpos, ypos);
}

void setMousePos(double xpos, double ypos)
{
    glfwSetCursorPos(windowInfo.mainWindow, xpos, ypos);
}

bool isWindowFocused()
{
    return windowInfo.isFocused;
}