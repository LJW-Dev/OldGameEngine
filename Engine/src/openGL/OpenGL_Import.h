#pragma once

#include <GL/glew.h>

GLuint imoprtTextureIntoGL(int width, int height, unsigned char* data);
GLuint loadDiskImageIntoGL(const char* imageName);