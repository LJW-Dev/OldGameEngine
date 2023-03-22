#include "Entity.h"
#include "src/openGL/OpenGL_Draw.h"

entityInfo entityList[MAX_ENTITIES];

void addEntity(float x, float y, float z)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (!entityList[i].isUsed)
        {
            entityList[i].isUsed = true;
            entityList[i].model = findAsset(XASSET_MODEL, "assets\\models\\cube.model").XModel;
            entityList[i].texture = loadMaterialAssetIntoGL("assets\\materials\\cube_texture.bmp");

            entityList[i].xPos = x;
            entityList[i].yPos = y;
            entityList[i].zPos = z;

            entityList[i].xRot = 0.0f;
            entityList[i].yRot = 0.0f;
            entityList[i].zRot = 0.0f;

            entityList[i].xScale = 1.0f;
            entityList[i].yScale = 1.0f;
            entityList[i].zScale = 1.0f;

            return;
        }
    }
}

void drawEntities()
{
    drawEntities(entityList);
}