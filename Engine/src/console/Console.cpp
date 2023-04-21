#include "src/commands/commands.h"
#include "src/input/textBox.h"

#include <GLFW/glfw3.h>

struct s_console
{
	bool isEnabled;
	bool isOpen;

	int textBox;
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
		if (isOpen)
		{
			tb_setFocus(console.textBox, true);
			tb_shouldRender(console.textBox, true);
		}
		else
		{
			tb_setFocus(console.textBox, false);
			tb_shouldRender(console.textBox, false);
		}

		console.isOpen = isOpen;
	}
}

void con_executeCommand()
{
	command_send(tb_getText(console.textBox));
	tb_resetText(console.textBox);
}

void con_close()
{
	con_setState(false);
}

void con_initConsole()
{
	console.isEnabled = true;
	console.isOpen = false;

	console.textBox = tb_init(con_executeCommand, GLFW_KEY_GRAVE_ACCENT, con_close, 10, 500, 30, "assets\\fonts\\font.bin");
}

void con_drawConsole()
{
	if (!con_isOpen())
		return;
}