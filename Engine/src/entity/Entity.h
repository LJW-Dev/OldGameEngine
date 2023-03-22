#pragma once

#include "src\assetDB\AssetDB.h"
#include "src/openGL/OpenGL_Import.h"

#define MAX_ENTITIES 30

struct entityInfo
{
    bool isUsed;

    XModel* model;
    GLuint texture;
    float xPos;
    float yPos;
    float zPos;

    float xRot;
    float yRot;
    float zRot;

    float xScale;
    float yScale;
    float zScale;
};

void addEntity(float x, float y, float z);
void drawEntities();