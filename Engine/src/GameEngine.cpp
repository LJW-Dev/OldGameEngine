#include "src\window\Window.h"
#include "src\openGL\OpenGL.h" 
#include "src\openGL\OpenGL_Draw.h" 
#include "src\assetDB\AssetDB.h"
#include "src\physics\Physics.h"
#include "src/input/Input.h"

#include "src/player/player.h"
#include "src/console/Console.h"
#include "src/scripting/script.h"

#include "src/dvar/dvar_list.h"
#include "src/debug/debug.h"

#include "src/error/error.h"

//static_cast<>()
//  checked safe conversion between types
//
//reinterpret_cast<>()
//  telling compiler "trust me: I know this doesn't look like a foo, but it is"

void mainLoop()
{
    while (!shouldExitWindow())
    {
        double deltaTime = startFrame();

        if (isWindowFocused())
        {
            updateInputs();
            updateCameraAngles(deltaTime);
        }
        
        scr_runCurrentThreads();

        updatePlayerPhysics(deltaTime);

        endFrame();
    }
}

int main()
{
    printf("Hello World!\n");

    initDvars();

    initWindow();

    initAssetPool();

    con_initConsole();

    initPlayerStruct();

    initWorldPhysics();

    initOpenGL();

    initScriptVM();

    initDebug();

    executeScript("assets\\scripts\\main.script");

    if (setjmp(mainErrorBuf) == 0)
        mainLoop();

    printf("Execution finished\n");
    return 0;
}