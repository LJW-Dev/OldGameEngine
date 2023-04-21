#include "src/error/error.h"
#include "src\openGL\OpenGL_Draw.h"
#include "src/assetDB/AssetDB.h"
#include "textBox.h"

#include <GLFW/glfw3.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEXT_BOXES 10

struct s_textBox
{
	bool isUsed;
	bool shouldRender;

	int endFocusKey;
	void (*endFocusCallback)();

	void (*onEnterCallback)();

	int bufferPos;
	char buffer[1000];

	fontAsset* font;
	int x;
	int y;
	int size;
};

struct s_textBoxInfo
{
	int focusedTextIndex;
	s_textBox* focusedText;

	s_textBox textBoxArray[MAX_TEXT_BOXES];
};

s_textBoxInfo textBoxInfo;

void tb_setFocus(int textBoxIndex, bool isFocused)
{
	if (isFocused)
	{
		textBoxInfo.focusedTextIndex = textBoxIndex;
		textBoxInfo.focusedText = &textBoxInfo.textBoxArray[textBoxIndex];
	}
	else
	{
		if (textBoxIndex == textBoxInfo.focusedTextIndex)
		{
			textBoxInfo.focusedTextIndex = -1;
			textBoxInfo.focusedText = NULL;
		}
	}
}

void tb_removeCurrFocus()
{
	textBoxInfo.focusedTextIndex = -1;
	textBoxInfo.focusedText = NULL;
}

bool tb_hasFocus()
{
	return textBoxInfo.focusedText != NULL;
}

void tb_shouldRender(int textBoxIndex, bool shouldRender)
{
	textBoxInfo.textBoxArray[textBoxIndex].shouldRender = shouldRender;
}

char* tb_getText(int textBoxIndex)
{
	return textBoxInfo.textBoxArray[textBoxIndex].buffer;
}

void tb_resetText(int textBoxIndex)
{
	textBoxInfo.textBoxArray[textBoxIndex].bufferPos = 0;
	memset(textBoxInfo.textBoxArray[textBoxIndex].buffer, '\0', sizeof(textBoxInfo.textBoxArray[textBoxIndex].buffer));
}

int parseGLFWKeyToChar(int key, int modifier);

void tb_parseKeyStroke(int key, int action, int modifier)
{
	if (textBoxInfo.focusedText == NULL)
		return;

	if (action == GLFW_RELEASE || textBoxInfo.focusedText->bufferPos >= sizeof(textBoxInfo.focusedText->buffer) - 1)
		return;

	if (textBoxInfo.focusedText->endFocusKey == key)
	{
		textBoxInfo.focusedText->endFocusCallback();
		return;
	}

	switch (key)
	{
	case GLFW_KEY_BACKSPACE:
		if (textBoxInfo.focusedText->bufferPos > 0)
		{
			textBoxInfo.focusedText->bufferPos--;
			textBoxInfo.focusedText->buffer[textBoxInfo.focusedText->bufferPos] = '\0';
		}
		return;

	case GLFW_KEY_ENTER:
		textBoxInfo.focusedText->onEnterCallback();
		return;

	case GLFW_KEY_ESCAPE:
		textBoxInfo.focusedText->endFocusCallback();
		return;
	}

	key = parseGLFWKeyToChar(key, modifier);

	if (key > 0)
	{
		textBoxInfo.focusedText->buffer[textBoxInfo.focusedText->bufferPos] = key;
		textBoxInfo.focusedText->bufferPos++;
	}
}

int tb_init(void (*onEnterCallback)(), int endFocusKey, void (*endFocusCallback)(), int x, int y, int size, const char* font)
{
	int textBoxIndex = -1;

	for (int i = 0; i < MAX_TEXT_BOXES; i++)
	{
		if (!textBoxInfo.textBoxArray[i].isUsed)
		{
			textBoxIndex = i;
			break;
		}
	}

	if (textBoxIndex == -1)
		error_exit("Exceeded %i textboxes.", MAX_TEXT_BOXES);

	s_textBox* tbPtr = &textBoxInfo.textBoxArray[textBoxIndex];
	tbPtr->isUsed = true;
	tbPtr->shouldRender = false;
	tbPtr->onEnterCallback = onEnterCallback;
	tbPtr->endFocusKey = endFocusKey;
	tbPtr->endFocusCallback = endFocusCallback;
	tbPtr->bufferPos = 0;
	tbPtr->x = x;
	tbPtr->y = y;
	tbPtr->size = size;
	tbPtr->font = findAsset(ASSET_FONT, "assets\\fonts\\font.bin", true).font;
	memset(tbPtr->buffer, '\0', sizeof(tbPtr->buffer));
	
	return textBoxIndex;
}

int tb_initDefaults(void (*onEnterCallback)(), int x, int y, int size, const char* font)
{
	return tb_init(onEnterCallback, GLFW_KEY_ESCAPE, tb_removeCurrFocus, x, y, size, font);
}

void tb_drawTextBoxes()
{
	for (int i = 0; i < MAX_TEXT_BOXES; i++)
	{
		s_textBox* tBox = &textBoxInfo.textBoxArray[i];
		if (tBox->isUsed && tBox->shouldRender)
		{
			drawText2D(tBox->font, tBox->buffer, tBox->x, tBox->y, tBox->size);
		}
	}
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