#include <GLFW/glfw3.h>
#include "src/utils/utils.h"

#include "src/assetDB/AssetDB.h"
#include "src\OpenGL.h"
#include  <string.h>
#include "src/OpenGL.h"
#include "src/dvar/dvar.h"

struct s_console
{
	bool isOpen;

	XFont* consoleFont;
	unsigned int consoleTexture;

	char consoleBuffer[100];
	int conBufferPos;

	int x;
	int y;
	int size;
};

s_console console_s;

bool con_isOpen()
{
	return console_s.isOpen;
}

void con_setState(bool isOpen)
{
	console_s.isOpen = isOpen;
}

void con_executeCommand()
{
	int secondWordPos = 0;
	while (console_s.consoleBuffer[secondWordPos] != ' ')
		secondWordPos++;

	console_s.consoleBuffer[secondWordPos] = '\0';
	secondWordPos++;
	char* secondWord = &console_s.consoleBuffer[secondWordPos];

	s_dvar* dvar = dvar_findVar(console_s.consoleBuffer);

	if (dvar != NULL)
	{
		switch (dvar_getType(dvar))
		{
		case DVAR_BOOL:
			dvar_setBool(dvar, atol(secondWord) != 0);
			break;

		case DVAR_FLOAT:
			dvar_setFloat(dvar, atof(secondWord));
			break;

		case DVAR_INTEGER:
			dvar_setInt(dvar, atol(secondWord));
			break;
		}
	}
}

void con_parseKeyStroke(int key, int action, int modifier)
{
	if (action == GLFW_RELEASE || console_s.conBufferPos >= sizeof(console_s.consoleBuffer) - 1)
		return;

	switch (key)
	{
	case GLFW_KEY_GRAVE_ACCENT:
		console_s.isOpen = false;
		return;

	case GLFW_KEY_BACKSPACE:
		if(console_s.conBufferPos > 0)
		{
			console_s.conBufferPos--;
			console_s.consoleBuffer[console_s.conBufferPos] = '\0';
		}
		return;

	case GLFW_KEY_ENTER:
		con_executeCommand();
		console_s.conBufferPos = 0;
		memset(console_s.consoleBuffer, '\0', sizeof(console_s.consoleBuffer));
		return;
	}
	
	key = parseGLFWKeyToChar(key, modifier);

	if(key > 0)
	{
		console_s.consoleBuffer[console_s.conBufferPos] = key;
		console_s.conBufferPos++;
	}
	
}

void initConsole()
{
	console_s.isOpen = false;
	console_s.consoleFont = findAsset(XASSET_FONT, "assets\\fonts\\font.bin").XFont;
	console_s.consoleTexture = loadMaterialIntoGL("assets\\materials\\console.bmp");
	console_s.conBufferPos = 0;
	memset(console_s.consoleBuffer, '\x0', sizeof(console_s.consoleBuffer));

	console_s.x = 10;
	console_s.y = 500;
	console_s.size = 30;
}

void drawConsole()
{
	printText2D(console_s.consoleBuffer, console_s.x, console_s.y, console_s.size);
}