#pragma once

#include "src\openGL\compile\StandardOpenGL.h"

GLuint imoprtTextureIntoGL(int width, int height, unsigned char* data);
GLuint loadMaterialAssetIntoGL(const char* materialName);
GLuint loadDiskImageIntoGL(const char* imageName);