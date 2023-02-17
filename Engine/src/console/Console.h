#pragma once

bool con_isOpen();
void con_parseKeyStroke(int key, int action, int modifier);
void initConsole();
void drawConsole();
void con_setState(bool isOpen);