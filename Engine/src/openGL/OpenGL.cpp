#include "src/openGL/OpenGL_Draw.h"
#include "src\window\Window.h"
#include "src/dvar/dvar_list.h"
#include "src\physics\Physics.h"
#include "src/console/Console.h"
#include "src/input/textBox.h"
#include "src/utils/time.h"
#include "src/debug/debug.h"
#include "src/world/world.h"

struct s_frameInfo
{
    double frameStartTime;
    double frameDeltaTime;
};

s_frameInfo frameInfo;

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

    world_setupWorld();
}

double startFrame()
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
    resetFrame();

    updateViewMatrix();

    drawScene();

    swapBuffersAndPoll();
}

void draw3D()
{
    world_drawWorld();

    drawEntities();
}

void draw2D()
{
    con_drawConsole();

    tb_drawTextBoxes();

    drawFPS(frameInfo.frameDeltaTime);
}

void drawScene()
{
    draw3D();
    draw2D();
}

void setWireframe(bool isOn)
{
    if(isOn)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}