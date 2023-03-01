#include "Input.h"
#include "src/window/Window.h"
#include "src\dvar\dvar.h"
#include "src/console/Console.h"

#include "src\player\Player.h"

#define INPUT_BUF_MAX 10

struct keyPress
{
    int key;
    int action;
    int modifier;
};

keyPress inputBuffer[INPUT_BUF_MAX];
int inputBufferSize = 0;

bool lockInputBuffer = false;

void glfw_keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    while (lockInputBuffer)
    {
    
    }

    if (inputBufferSize >= INPUT_BUF_MAX)
        return;

    inputBuffer[inputBufferSize].key = key;
    inputBuffer[inputBufferSize].action = action;
    inputBuffer[inputBufferSize].modifier = mods;

    inputBufferSize++;
}

void resetKeyState()
{
    playerStruct.keyState.backwardsKey = false;
    playerStruct.keyState.forwardKey = false;
    playerStruct.keyState.leftKey = false;
    playerStruct.keyState.rightKey = false;
    playerStruct.keyState.jumpKey = false;
}

void updateInputs()
{
    getMousePos(&playerStruct.keyState.mouse_x, &playerStruct.keyState.mouse_y);
    setMousePos(WINDOW_X / 2, WINDOW_Y / 2);
    
    lockInputBuffer = true;

    bool isConsoleOpen = con_isOpen();
    
    for (int i = 0; i < inputBufferSize; i++)
    {
        if (isConsoleOpen)
        {
            con_parseKeyStroke(inputBuffer[i].key, inputBuffer[i].action, inputBuffer[i].modifier);
            continue;
        }

        bool keyResult = (inputBuffer[i].action == GLFW_PRESS || inputBuffer[i].action == GLFW_REPEAT);

        switch (inputBuffer[i].key)
        {
        case GLFW_KEY_W:
            playerStruct.keyState.forwardKey = keyResult;
            break;

        case GLFW_KEY_S:
            playerStruct.keyState.backwardsKey = keyResult;
            break;

        case GLFW_KEY_D:
            playerStruct.keyState.rightKey = keyResult;
            break;

        case GLFW_KEY_A:
            playerStruct.keyState.leftKey = keyResult;
            break;

        case GLFW_KEY_SPACE:
            playerStruct.keyState.jumpKey = keyResult;
            break;

        case GLFW_KEY_ESCAPE:
            dvar_setBool(game_exit, true);
            break;

        case GLFW_KEY_GRAVE_ACCENT:

            if (keyResult)
            {
                resetKeyState();
                con_setState(true);
                isConsoleOpen = true;
            }
            break;
        }


    }
    inputBufferSize = 0;

    lockInputBuffer = false;
}



