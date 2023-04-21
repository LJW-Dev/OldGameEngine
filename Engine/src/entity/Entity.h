#pragma once

#include "src\assetDB\AssetDB.h"

#define MAX_ENTITIES 30

struct entityInfo
{
    bool isUsed;

    modelAsset* model;
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

entityInfo* addEntity(float x, float y, float z, const char* model);
void drawEntities();