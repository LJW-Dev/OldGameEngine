#pragma once

struct s_keyboardInfo
{
	int closeKey;
	void (*onClosedCallback)();

	void (*onEnterCallback)();

	int bufferPos;
	char buffer[1000];
};

void kb_parseKeyStroke(int key, int action, int modifier);

void kb_initDefaults(s_keyboardInfo* kbInfo, void (*onEnterCallback)());
void kb_init(s_keyboardInfo* kbInfo);
void kb_setFocus(s_keyboardInfo* kbInfo);
bool kb_hasFocus();
void kb_removeFocus();
void kb_resetBuffer();