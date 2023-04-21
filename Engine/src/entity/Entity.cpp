#include "Entity.h"
#include "src/openGL/OpenGL_Draw.h"
#include "src/error/error.h"

entityInfo entityList[MAX_ENTITIES];

entityInfo* addEntity(float x, float y, float z, const char* model)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (!entityList[i].isUsed)
        {
            entityList[i].isUsed = true;
            entityList[i].model = findAsset(ASSET_MODEL, model, true).model;

            entityList[i].xPos = x;
            entityList[i].yPos = y;
            entityList[i].zPos = z;

            entityList[i].xRot = 0.0f;
            entityList[i].yRot = 0.0f;
            entityList[i].zRot = 0.0f;

            entityList[i].xScale = 1.0f;
            entityList[i].yScale = 1.0f;
            entityList[i].zScale = 1.0f;

            return &entityList[i];
        }
    }

    error_noexit("addEntity: No more space.");

    return NULL;
}

void drawEntities()
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entityList[i].isUsed)
            drawEntity(entityList);
    }
}