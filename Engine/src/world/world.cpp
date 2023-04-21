#include "src/openGL/OpenGL_Draw.h"

worldAsset* worldObj;

void world_setupWorld()
{
    worldObj = findAsset(ASSET_WORLD, "assets\\world\\world.model", true).world;
}

void world_drawWorld()
{
    drawWorld(worldObj);
}