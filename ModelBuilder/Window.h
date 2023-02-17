#pragma once

#include "StandardOpenGL.h"

GLFWwindow* initWindow();
void swapBuffersAndPoll();
bool shouldExitWindow();
void getMousePos(double* xpos, double* ypos);
void setMousePos(double xpos, double ypos);
int getKeyboardInput(int key);