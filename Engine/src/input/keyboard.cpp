#include <string.h>
#include <GLFW/glfw3.h>
#include <ctype.h>

#include "keyboard.h"

s_keyboardInfo* focusedText = NULL;

int parseGLFWKeyToChar(int key, int modifier);

void kb_setFocus(s_keyboardInfo* kbInfo)
{
	focusedText = kbInfo;
}

void kb_removeFocus()
{
	focusedText = NULL;
}

bool kb_hasFocus()
{
	return focusedText != NULL;
}

void kb_resetBuffer()
{
	focusedText->bufferPos = 0;
	memset(focusedText->buffer, '\0', sizeof(focusedText->buffer));
}

void kb_parseKeyStroke(int key, int action, int modifier)
{
	if (focusedText == NULL)
		return;

	if (action == GLFW_RELEASE || focusedText->bufferPos >= sizeof(focusedText->buffer) - 1)
		return;

	if (focusedText->closeKey == key)
	{
		focusedText->onClosedCallback();
		return;
	}

	switch (key)
	{
	case GLFW_KEY_BACKSPACE:
		if (focusedText->bufferPos > 0)
		{
			focusedText->bufferPos--;
			focusedText->buffer[focusedText->bufferPos] = '\0';
		}
		return;

	case GLFW_KEY_ENTER:
		focusedText->onEnterCallback();
		return;

	case GLFW_KEY_ESCAPE:
		focusedText->onClosedCallback();
		return;
	}

	key = parseGLFWKeyToChar(key, modifier);

	if (key > 0)
	{
		focusedText->buffer[focusedText->bufferPos] = key;
		focusedText->bufferPos++;
	}
}

void kb_init(s_keyboardInfo* kbInfo)
{
	kbInfo->bufferPos = 0;
	memset(kbInfo->buffer, '\0', sizeof(kbInfo->buffer));
}

void kb_initDefaults(s_keyboardInfo* kbInfo, void (*onEnterCallback)())
{
	kbInfo->onEnterCallback = onEnterCallback;

	kbInfo->closeKey = GLFW_KEY_UNKNOWN;
	kbInfo->onClosedCallback = kb_removeFocus;

	kb_init(kbInfo);
}

char USKeyboardAltKeys[256] =
{
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',

	' ', ' ', ' ', ' ', ' ', ' ', ' ', '"', ' ', ' ', ' ', ' ', '<', '_', '>', '?',
	')', '!', '@', '#', '$', '%', '^', '&', '*', '(', ' ', ':', ' ', '+', ' ', ' ',

	'~', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '{', '|', '}', ' ', ' ',

	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',

	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',

	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',

	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',

	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
};

int parseGLFWKeyToChar(int key, int modifier)
{
	if (key > GLFW_KEY_WORLD_2 || key < GLFW_KEY_SPACE)
		return -1;

	if (isalpha(key))
	{
		if ((modifier & GLFW_MOD_SHIFT) != GLFW_MOD_SHIFT)
			key += 0x20;
	}
	else
	{
		if ((modifier & GLFW_MOD_SHIFT) == GLFW_MOD_SHIFT)
			key = USKeyboardAltKeys[key];
	}

	return key;
}