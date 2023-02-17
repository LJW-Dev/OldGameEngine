#include "StandardOpenGL.h"

GLFWwindow* mainWindow;

GLFWwindow* initWindow()
{
    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return NULL;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 

    // Open a window and create its OpenGL context
    mainWindow = glfwCreateWindow(1024, 768, "Tutorial 01", NULL, NULL);
    if (mainWindow == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(mainWindow); // Initialize GLEW
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return NULL;
    }

    // Ensure we can capture the escape key being pressed below
    //glfwSetInputMode(mainWindow, GLFW_STICKY_KEYS, GL_TRUE);

    return mainWindow;
}

void swapBuffersAndPoll()
{
    // Swap buffers
    glfwSwapBuffers(mainWindow);
    glfwPollEvents();
}

bool shouldExitWindow()
{
    return glfwGetKey(mainWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(mainWindow) == 0;
}

void getMousePos(double* xpos, double* ypos)
{
    glfwGetCursorPos(mainWindow, xpos, ypos);
}

void setMousePos(double xpos, double ypos)
{
    glfwSetCursorPos(mainWindow, xpos, ypos);
}

int getKeyboardInput(int key)
{
    return glfwGetKey(mainWindow, key);
}