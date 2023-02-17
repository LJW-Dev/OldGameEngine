#include "StandardOpenGL.h"

#include "Window.h"
#include "OpenGL.h" 

void mainLoop()
{
    do
    {
        drawScene();

        swapBuffersAndPoll();

    } // Check if the ESC key was pressed or the window was closed
    while (shouldExitWindow());
}

int main()
{
    std::cout << "Hello World!\n";

    initWindow();

    initOpenGL();



    mainLoop();
}