#pragma once

#include "src\openGL\OpenGL.h"
#include "src\assetDB\AssetDB.h"
#include "src/entity/Entity.h"

void setupShaders();
void drawWorld(XworldObject* worldObj);
void printText2D(XFont* font, const char* text, int x, int y, int size);
void drawImage2D(GLuint texture, int x, int y, int size);
void drawEntities(entityInfo* entArray);