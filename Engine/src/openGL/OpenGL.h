#pragma once
#include "src\openGL\compile\StandardOpenGL.h"

void initOpenGL();
void drawScene();

float startFrame();
void endFrame();

void setWireframe(bool isOn);