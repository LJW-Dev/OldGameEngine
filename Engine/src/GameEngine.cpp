#include "src\openGL\compile\StandardOpenGL.h"

#include "src\window\Window.h"
#include "src\OpenGL.h" 
#include "src\assetDB\AssetDB.h"
#include "src\physics\Physics.h"
#include "src/input/Input.h"
#include "src/dvar/dvar.h"
#include "src/player/player.h"
#include "src/console/Console.h"
#include "src/scripting/script.h"

//static_cast<>()
//  checked safe conversion between types
//
//reinterpret_cast<>()
//  telling compiler "trust me: I know this doesn't look like a foo, but it is"

void mainLoop()
{
    double currentTime, deltaTime;
    double lastTime = glfwGetTime();

    do 
    {
        currentTime = glfwGetTime();

        if (dvar_getBool(sv_overrideTimestep))
            deltaTime = dvar_getFloat(sv_timestep);
        else
            deltaTime = float(currentTime - lastTime);

        lastTime = currentTime;

        if (isWindowFocused())
        {
            updateInputs();
            updateCameraAngles(deltaTime);
        }

        scr_runCurrentThreads();

        updatePlayerPhysics(deltaTime);

        updateViewMatrix();

        drawScene(deltaTime);

        swapBuffersAndPoll();

    } // Check if the ESC key was pressed or the window was closed
    while (!shouldExitWindow());

}

int main()
{
    std::cout << "Hello World!\n";

    initDvars();

    initWindow();

    initXAssetPool();

    initConsole();

    initPlayerStruct();

    initWorldPhysics();

    initOpenGL();

    initScriptVM();

    executeScript("assets\\scripts\\main.script");

    mainLoop();
}