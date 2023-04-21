#pragma once

#include "script.h"

#define SCR_MAX_VARS 20
#define SCR_MAX_STACK 10
#define SCR_MAX_CALLS 20
#define SCR_MAX_THREADS 10

struct localFunction
{
	s_scriptVar vars[SCR_MAX_VARS];

	char* callReturnPos;
};

struct scrThreadInfo
{
	bool isThreadActive;
	char* scriptPos;

	s_scriptVar* freeStack;
	s_scriptVar* maxStack;
	s_scriptVar stack[SCR_MAX_STACK];

	localFunction* currFunc;
	localFunction* maxFunc;
	localFunction functionStack[SCR_MAX_CALLS];

	bool isThreadPaused;
	double pauseStart;
	double pauseTime;

	char callParamCount;
};

struct scriptVM
{
	int currThreadIndex;
	scrThreadInfo* currThread;
	scrThreadInfo threads[SCR_MAX_THREADS];

	int scriptStringArrayPos;
	char scriptStringArray[1000];
};

extern scriptVM scrVM;

void scr_pushToStack(s_scriptVar data);
s_scriptVar scr_popFromStack();
void scr_setVar(int index, s_scriptVar val);
s_scriptVar scr_getVar(int index);
void addNewThread(char* scriptPos, char paramCount);
void scr_killCurrThread();
char* scr_addString(char* str);
void popFunctionParams(char paramCount, int threadIndex);