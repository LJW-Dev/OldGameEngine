#include "src/assetDB/AssetDB.h"
#include "script_vm.h"
#include "script_read.h"
#include "script_func.h"
#include "src/error/error.h"

#include "src/utils/time.h"
#include <string>

scriptVM scrVM;

void scr_killCurrThread()
{
	printf("Thread %i killed\n", scrVM.currThreadIndex);
	scrVM.currThread->isThreadActive = false;
}

void scr_error(const char* msg)
{
	scr_killCurrThread();
	error_noexit("Script Error: %s", msg);
}

void scr_threadStackCheck(int threadIndex)
{
	if ((&scrVM.threads[threadIndex])->freeStack > (&scrVM.threads[threadIndex])->maxStack)
	{
		error_exit("scr_stackCheck: Max stack reached!\n");
		return;
	}
	else if ((&scrVM.threads[threadIndex])->freeStack < (&scrVM.threads[threadIndex])->stack)
	{
		error_exit("scr_stackCheck: Min stack reached!\n");
		return;
	}
}

void scr_pushToThreadStack(int threadIndex, s_scriptVar data)
{
	(&scrVM.threads[threadIndex])->freeStack->type = data.type;
	(&scrVM.threads[threadIndex])->freeStack->val = data.val;

	(&scrVM.threads[threadIndex])->freeStack++;

	scr_threadStackCheck(threadIndex);
}

void scr_pushToStack(s_scriptVar data)
{
	scr_pushToThreadStack(scrVM.currThreadIndex, data);
	scr_threadStackCheck(scrVM.currThreadIndex);
}

void scr_stackCheck()
{
	scr_threadStackCheck(scrVM.currThreadIndex);
}

s_scriptVar scr_popFromStack()
{
	scrVM.currThread->freeStack--;
	scr_stackCheck();

	return *scrVM.currThread->freeStack;
}

// TODO: work on, change name etc
void popFunctionParams(char paramCount, int threadIndex)
{
	if (paramCount > SCR_MAX_VARS)
	{
		scr_error("Script call exceeded SCR_MAX_VARS parameters");
		return;
	}

	for (int i = 0; i < paramCount; i++)
	{
		(&scrVM.threads[threadIndex])->currFunc->vars[i] = scr_popFromStack();
	}
}

void scr_setVar(int index, s_scriptVar val)
{
	if (index > (SCR_MAX_VARS - 1))
	{
		error_exit("scr_setVar: Index greater than %i\n", (SCR_MAX_VARS - 1));
		return;
	}


	scrVM.currThread->currFunc->vars[index].type = val.type;
	scrVM.currThread->currFunc->vars[index].val = val.val;
}

s_scriptVar scr_getVar(int index)
{
	if (index > (SCR_MAX_VARS - 1))
	{
		error_exit("scr_getVar: Index greater than %i\n", (SCR_MAX_VARS - 1));
	}

	return scrVM.currThread->currFunc->vars[index];
}

char* scr_addString(char* str)
{
	int strLen = strlen(str) + 1;

	if (scrVM.scriptStringArrayPos + strLen > sizeof(scrVM.scriptStringArray))
	{
		error_exit("scr_addString: Not enough space in scriptStringArray for %s.\n", str);
	}
	
	char* newStrPos = scrVM.scriptStringArray + scrVM.scriptStringArrayPos;
	strcpy_s(newStrPos, strLen, str);

	scrVM.scriptStringArrayPos += strLen;

	return newStrPos;
}

s_scriptVar scr_popVar()
{
	return scr_popFromStack();
}

char scr_popBool()
{
	s_scriptVar var = scr_popFromStack();

	if (var.type == SCRIPT_BOOL)
		return var.val.t_bool;

	switch (var.type)
	{
	case SCRIPT_INTEGER:
		return var.val.t_int != 0;

	default:
		error_exit("scr_popBool: cannot cast to bool!\n");
	}
}

int scr_popInteger()
{
	s_scriptVar var = scr_popFromStack();

	if (var.type == SCRIPT_INTEGER)
		return var.val.t_int;

	switch (var.type)
	{
	//case SCRIPT_STRING:
	//	return atoi(var.val.t_str);

	case SCRIPT_FLOAT:
		return (int)var.val.t_float;

	default:
		error_exit("scr_popInteger: cannot cast to integer!\n");
	}
}

float scr_popFloat()
{
	s_scriptVar var = scr_popFromStack();

	if (var.type == SCRIPT_FLOAT)
		return var.val.t_float;

	switch (var.type)
	{
	//case SCRIPT_STRING:
	//	return atof(var.val.t_str);

	case SCRIPT_INTEGER:
		return (float)var.val.t_int;

	default:
		error_exit("scr_popFloat: cannot cast to float!\n");
	}
}

char* scr_popString()
{
	s_scriptVar var = scr_popFromStack();

	if(var.type == SCRIPT_STRING)
		return var.val.t_str;

	error_exit("scr_popString: cannot cast to string!\n");
}

void scr_pushVar(s_scriptVar var)
{
	scr_pushToStack(var);
}

void scr_pushNull()
{
	s_scriptVar var;
	var.type = SCRIPT_NULL;
	var.val.t_data = NULL;

	scr_pushToStack(var);
}

void scr_pushBool(bool value)
{
	s_scriptVar var;
	var.type = SCRIPT_BOOL;
	var.val.t_bool = value;

	scr_pushToStack(var);
}

void scr_pushInteger(int value)
{
	s_scriptVar var;
	var.type = SCRIPT_INTEGER;
	var.val.t_int = value;

	scr_pushToStack(var);
}

void scr_pushFloat(float value)
{
	s_scriptVar var;
	var.type = SCRIPT_FLOAT;
	var.val.t_float = value;

	scr_pushToStack(var);
}

void scr_pushString(char* value)
{
	s_scriptVar var;
	var.type = SCRIPT_FLOAT;
	var.val.t_str = scr_addString(value);

	scr_pushToStack(var);
}

void initScriptVM()
{
	
}

void addNewThread(char* scriptPos, char paramCount)
{
	for (int i = 0; i < SCR_MAX_THREADS; i++)
	{
		if (!scrVM.threads[i].isThreadActive)
		{
			scrVM.threads[i].isThreadActive = true;
			scrVM.threads[i].isThreadPaused = false;

			scrVM.threads[i].scriptPos = scriptPos;
			
			scrVM.threads[i].freeStack = scrVM.threads[i].stack;
			scrVM.threads[i].maxStack = &scrVM.threads[i].stack[SCR_MAX_STACK - 1];

			scrVM.threads[i].currFunc = scrVM.threads[i].functionStack;
			scrVM.threads[i].maxFunc = &scrVM.threads[i].functionStack[SCR_MAX_CALLS - 1];

			popFunctionParams(paramCount, i);
			
			return;
		}
	}

	error_noexit("Unable to exec new thread, more than %i threads executing currently\n", SCR_MAX_THREADS);
}

void execCurrThread()
{
	if(scrVM.currThread == NULL)
	{
		error_exit("scrVM.currThread was null!\n");
	}

	if (scrVM.currThread->isThreadPaused)
	{
		if (scrVM.currThread->pauseStart + scrVM.currThread->pauseTime > time_Milliseconds())
			return;
		else
			scrVM.currThread->isThreadPaused = false;
	}

	while (!scrVM.currThread->isThreadPaused && scrVM.currThread->isThreadActive)
	{
		char opcode = *scrVM.currThread->scriptPos;
		scrVM.currThread->scriptPos++;

		if (opcode >= e_opcodeMax)
			scr_error("opcode was greater than e_opcodeMax!");

		printf("%i\n", opcode);

		opcodeFuncArray[opcode]();
	}
}

void scr_runCurrentThreads()
{
	for (int i = 0; i < SCR_MAX_THREADS; i++)
	{
		if (scrVM.threads[i].isThreadActive)
		{
			scrVM.currThreadIndex = i;
			scrVM.currThread = &scrVM.threads[i];
			execCurrThread();
		}
	}
}

struct scriptHeader
{
	int stringTablePtr;
	int stringTableCount;

	int exportTablePtr;
	int exportTableCount;

	int importTablePtr;
	int importTableCount;
};

void linkScript(scriptAsset* script)
{
	scriptHeader* header = (scriptHeader*)script->script;
	char* scriptStart = script->script;

	char* currPos = scriptStart + header->stringTablePtr;
	for (int i = 0; i < header->stringTableCount; i++)
	{
		char* stringPtr = scr_addString(currPos);
		currPos += strlen(currPos) + 1;

		int usedCount = *(int*)currPos;
		currPos += sizeof(int);

		for (int j = 0; j < usedCount; j++)
		{
			char** instructionPtr = (char**)(scriptStart + *(int*)currPos);
			currPos += sizeof(int);

			*instructionPtr = stringPtr;
		}
	}

	currPos = scriptStart + header->exportTablePtr;
	for (int i = 0; i < header->exportTableCount; i++)
	{
		currPos += strlen(currPos) + 1;

		int bytecodePtr = *(int*)currPos;
		currPos += sizeof(int);

		currPos += sizeof(int);

		if (*currPos == 1)
		{
			addNewThread(scriptStart + bytecodePtr, 0);
		}
		currPos += sizeof(char);
	}

	currPos = scriptStart + header->importTablePtr;
	for (int i = 0; i < header->importTableCount; i++)
	{
		scrFuncPtr builtin = doesFunctionExist(currPos);
		if (builtin == NULL)
			error_exit("No builtin function called %s", currPos);

		currPos += strlen(currPos) + 1;

		int usedCount = *(int*)currPos;
		currPos += sizeof(int);

		for (int j = 0; j < usedCount; j++)
		{
			scrFuncPtr* instructionPtr = (scrFuncPtr*)(scriptStart + *(int*)currPos);
			currPos += sizeof(int);

			*instructionPtr = builtin;
		}
	}
}

void executeScript(const char* scrName)
{
	scriptAsset* script = findAsset(ASSET_SCRIPT, scrName, false).script;

	if (script == NULL)
	{
		error_noexit("cant find script %s\n", scrName);
		return;
	}

	linkScript(script);
}