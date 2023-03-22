#include "src/openGL/OpenGL_Draw.h"
#include "src\openGL\compile\ShaderCompile.h"
#include "src\window\Window.h"

#include "src/dvar/dvar_list.h"

#include "src\physics\Physics.h"
#include "src/console/Console.h"

#include "src/player/Player.h"

#include <math.h>

#include <vector>

#include "src\assetDB\AssetDB.h"
#include "src/entity/Entity.h"

#include "src/utils/time.h"

#include "src/debug/debug.h"

struct s_frameInfo
{
    double frameStartTime;
    double frameDeltaTime;
};

s_frameInfo frameInfo;

XworldObject* worldObj;

void setupWorld()
{
    worldObj = findAsset(XASSET_WORLD, "assets\\world\\world.model").World;
}

void setupOpenGLSettings()
{
    glEnable(GL_DEPTH_TEST); // Enable depth test
    glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
    glEnable(GL_CULL_FACE); // Cull triangles which normal is not towards the camera
}

void initFrameStruct()
{
    frameInfo.frameStartTime = time_Milliseconds();
    frameInfo.frameDeltaTime = 0;
}

void initOpenGL()
{
    initFrameStruct();

    setupShaders();

    setupOpenGLSettings();

    setupWorld();
}

float startFrame()
{
    double currentTime, deltaTime;

    currentTime = time_Milliseconds();

    if (dvar_getBool(sv_overrideTimestep))
        deltaTime = dvar_getFloat(sv_timestep);
    else
        deltaTime = float(currentTime - frameInfo.frameStartTime); // still has old start time


    frameInfo.frameDeltaTime = deltaTime;
    frameInfo.frameStartTime = currentTime;

    return deltaTime;
}

void endFrame()
{
    updateViewMatrix();

    drawScene();

    swapBuffersAndPoll();
}

//double lastTime = 0;
void drawScene()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawWorld(worldObj);

    drawEntities();
    
    if (con_isOpen())
        drawConsole();

    drawFPS(frameInfo.frameDeltaTime);
}

void setWireframe(bool isOn)
{
    if(isOn)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}