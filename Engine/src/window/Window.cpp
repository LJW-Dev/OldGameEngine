#include <GL/glew.h>
#include <stdio.h>

#include "src/window/Window.h"
#include "src/input/Input.h"
#include "src/dvar/dvar.h"

GLFWwindow* mainWindow;

void initCallbacks()
{
    glfwSetKeyCallback(mainWindow, glfw_keyPressCallback);
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
    mainWindow = glfwCreateWindow(WINDOW_X, WINDOW_Y, "Engine", NULL, NULL);
    if (mainWindow == NULL) 
    {
        printf("Failed to open GLFW window.\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(mainWindow); // Initialize GLEW

    if (glewInit() != GLEW_OK) 
    {
        printf("Failed to initialize GLEW.\n");
        return false;
    }

    // Ensure we can capture the escape key being pressed below
    //glfwSetInputMode(mainWindow, GLFW_STICKY_KEYS, GL_TRUE);

    initCallbacks();

    return true;
}

void swapBuffersAndPoll()
{
    // Swap buffers
    glfwSwapBuffers(mainWindow);
    glfwPollEvents();
}

bool shouldExitWindow()
{
    return dvar_getBool(game_exit) || glfwWindowShouldClose(mainWindow);
}

void getMousePos(double* xpos, double* ypos)
{
    glfwGetCursorPos(mainWindow, xpos, ypos);
}

void setMousePos(double xpos, double ypos)
{
    glfwSetCursorPos(mainWindow, xpos, ypos);
}

bool isWindowFocused()
{
    return glfwGetWindowAttrib(mainWindow, GLFW_FOCUSED);
}