#pragma once

void tb_parseKeyStroke(int key, int action, int modifier);
void tb_drawTextBoxes();

int tb_initDefaults(void (*onEnterCallback)(), int x, int y, int size, const char* font);
int tb_init(void (*onEnterCallback)(), int endFocusKey, void (*endFocusCallback)(), int x, int y, int size, const char* font);
void tb_setFocus(int textBoxIndex, bool isFocused);
char* tb_getText(int textBoxIndex);
bool tb_hasFocus();
void tb_resetText(int textBoxIndex);

void tb_shouldRender(int textBoxIndex, bool shouldRender);