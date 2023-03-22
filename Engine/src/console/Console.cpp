#include "src/window/utils/utils.h"
#include "src\openGL\OpenGL_Draw.h"
#include "src/assetDB/AssetDB.h"
#include "src/commands/commands.h"
#include "src/input/keyboard.h"

#include  <string.h>
#include "src/dvar/dvar.h"

struct s_console
{
	bool isEnabled;
	bool isOpen;

	XFont* consoleFont;

	s_keyboardInfo kbInfo;

	int x;
	int y;
	int size;
};

s_console console;

bool con_isOpen()
{
	return console.isOpen;
}

void con_setState(bool isOpen)
{
	if (console.isEnabled)
	{
		console.isOpen = isOpen;

		if (isOpen)
			kb_setFocus(&console.kbInfo);
		else
			kb_removeFocus();
	}
}

void con_executeCommand()
{
	command_send(console.kbInfo.buffer);
	kb_resetBuffer();
}

void con_close()
{
	con_setState(false);
}

void initConsole()
{
	console.isEnabled = true;
	console.isOpen = false;
	console.consoleFont = findAsset(XASSET_FONT, "assets\\fonts\\font.bin").XFont;

	console.x = 10;
	console.y = 500;
	console.size = 30;

	console.kbInfo.closeKey = GLFW_KEY_GRAVE_ACCENT;
	console.kbInfo.onClosedCallback = con_close;
	console.kbInfo.onEnterCallback = con_executeCommand;

	kb_init(&console.kbInfo);
}

void drawConsole()
{
	printText2D(console.consoleFont, console.kbInfo.buffer, console.x, console.y, console.size);
}