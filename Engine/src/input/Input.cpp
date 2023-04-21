#include "Input.h"
#include "src/window/Window.h"
#include "src/dvar/dvar_list.h"
#include "src/console/Console.h"
#include "textBox.h"

#include "src/commands/commands.h"

#include "src\player\Player.h"

#include <GLFW/glfw3.h>

#include <thread>

#define INPUT_BUF_MAX 10

struct keyPress
{
    int key;
    int action;
    int modifier;
};

keyPress inputBuffer[INPUT_BUF_MAX];
int inputBufferSize = 0;

void input_keyPressCallback(int key, int action, int mods)
{
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

void updateKeyBind(int key, bool isKeyDown)
{
    switch (key)
    {
    case GLFW_KEY_W:
        playerStruct.keyState.forwardKey = isKeyDown;
        break;

    case GLFW_KEY_S:
        playerStruct.keyState.backwardsKey = isKeyDown;
        break;

    case GLFW_KEY_D:
        playerStruct.keyState.rightKey = isKeyDown;
        break;

    case GLFW_KEY_A:
        playerStruct.keyState.leftKey = isKeyDown;
        break;

    case GLFW_KEY_SPACE:
        playerStruct.keyState.jumpKey = isKeyDown;
        break;

    case GLFW_KEY_ESCAPE:
        command_send("game_exit");
        break;

    // `
    case GLFW_KEY_GRAVE_ACCENT:
        if (isKeyDown)
        {
            resetKeyState();
            con_setState(true);
        }
        break;
    }
}

void updateInputs()
{
    getMousePos(&playerStruct.keyState.mouse_x, &playerStruct.keyState.mouse_y);
    setMousePos(WINDOW_X / 2, WINDOW_Y / 2);
    
    bool textBoxFocused = tb_hasFocus();
    
    for (int i = 0; i < inputBufferSize; i++)
    {
        if (textBoxFocused)
        {
            tb_parseKeyStroke(inputBuffer[i].key, inputBuffer[i].action, inputBuffer[i].modifier);
            continue;
        }

        bool isKeyDown = (inputBuffer[i].action == GLFW_PRESS || inputBuffer[i].action == GLFW_REPEAT);

        updateKeyBind(inputBuffer[i].key, isKeyDown);
    }
    inputBufferSize = 0;
}



