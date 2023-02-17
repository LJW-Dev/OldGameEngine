#pragma once

void initOpenGL();
void drawScene(float deltaTime);

unsigned int loadMaterialIntoGL(const char* materialName);
unsigned int loadImageIntoGL(const char* imageName);

void printText2D(const char* text, int x, int y, int size);