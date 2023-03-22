#pragma once

bool initWindow();
void swapBuffersAndPoll();
bool shouldExitWindow();
void getMousePos(double* xpos, double* ypos);
void setMousePos(double xpos, double ypos);
bool isWindowFocused();

#define WINDOW_X 1024
#define WINDOW_Y 768

void window_windowShouldClose();
void window_setFocus(bool focused);