#pragma once

#include "src\openGL\OpenGL.h"
#include "src\assetDB\AssetDB.h"
#include "src/entity/Entity.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

void setupShaders();
void drawWorld(worldAsset* worldObj);
void drawText2D(fontAsset* font, const char* text, int x, int y, int size);
void drawImage2D(GLuint texture, int x, int y, int size);
void drawEntity(entityInfo* entArray);
void resetFrame();