#include "src/assetDB/AssetDB.h"
#include "script.h"
#include "script_func.h"
#include "src/error/error.h"

#include "src/utils/time.h"
#include <string>

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
	int currThreadNum;
	scrThreadInfo* currThread;
	scrThreadInfo threads[SCR_MAX_THREADS];
};

scriptVM scrVM;

void scr_killCurrThread()
{
	printf("Thread %i killed\n", scrVM.currThreadNum);
	scrVM.currThread->isThreadActive = false;
}

void scr_error(const char* msg, bool isTerminalError)
{
	scr_killCurrThread();

	if (isTerminalError)
		error_exit("Script Error: %s", msg);
	else
		error_noexit("Script Error: %s", msg);
}

void scr_stackCheck()
{
	if (scrVM.currThread->freeStack > scrVM.currThread->maxStack)
	{
		scr_error("Max stack reached!", true);
		return;
	}
	else if (scrVM.currThread->freeStack < scrVM.currThread->stack)
	{
		scr_error("Min stack reached!", true);
	}
}

void scr_pushToStack(s_scriptVar data)
{
	scr_stackCheck();
	
	scrVM.currThread->freeStack->type = data.type;
	scrVM.currThread->freeStack->val = data.val;
	scrVM.currThread->freeStack++;
}

s_scriptVar scr_popFromStack()
{
	scr_stackCheck();

	scrVM.currThread->freeStack--;
	return *scrVM.currThread->freeStack;
}

s_scriptVar scr_popVar()
{
	return scr_popFromStack();
}

int scr_popInteger()
{
	return scr_popFromStack().val.t_int;
}

float scr_popFloat()
{
	return scr_popFromStack().val.t_float;
}

char* scr_popString()
{
	return scr_popFromStack().val.t_str;
}

void scr_setVar(int index, s_scriptVar val)
{
	if (index > (SCR_MAX_VARS - 1))
		scr_error("scr_setVar index greater than (SCR_MAX_VARS - 1)", true);

	scrVM.currThread->currFunc->vars[index].type = val.type;
	scrVM.currThread->currFunc->vars[index].val = val.val;
}

s_scriptVar scr_getVar(int index)
{
	if (index > (SCR_MAX_VARS - 1))
		scr_error("scr_getVar index greater than (SCR_MAX_VARS - 1)", true);

	return scrVM.currThread->currFunc->vars[index];
}

int scr_peekInteger()
{
	return *(int*)scrVM.currThread->scriptPos;
}

char scr_readChar()
{
	char val = *(char*)scrVM.currThread->scriptPos;
	scrVM.currThread->scriptPos += sizeof(char);
	return val;
}

int scr_readInteger()
{
	int val = *(int*)scrVM.currThread->scriptPos;
	scrVM.currThread->scriptPos += sizeof(int);
	return val;
}

float scr_readFloat()
{
	float val = *(float*)scrVM.currThread->scriptPos;
	scrVM.currThread->scriptPos += sizeof(float);
	return val;
}

char* scr_readString()
{
	char* val = (char*)scrVM.currThread->scriptPos;
	scrVM.currThread->scriptPos += strlen(val) + 1;
	return val;
}

void op_pushFloat()
{
	s_scriptVar value;
	value.type = SCRIPT_FLOAT;
	value.val.t_float = scr_readFloat();

	scr_pushToStack(value);
}

void op_pushInt()
{
	s_scriptVar value;
	value.type = SCRIPT_INTEGER;
	value.val.t_int = scr_readInteger();

	scr_pushToStack(value);
}

void op_pushString()
{
	s_scriptVar value;
	value.type = SCRIPT_STRING;
	value.val.t_str = scr_readString();

	scr_pushToStack(value);
}

void op_jump()
{
	scrVM.currThread->scriptPos += scr_peekInteger();
}

void op_callBuiltin()
{
	scrFuncArray[scr_readInteger()].func();
}

void op_callScript()
{
	if (scrVM.currThread->currFunc == scrVM.currThread->maxFunc)
	{
		scr_error("Too many nested calls in one thread!", true);
	}

	int jumpVal = scr_peekInteger();

	scrVM.currThread->currFunc->callReturnPos = scrVM.currThread->scriptPos + sizeof(int);
	
	scrVM.currThread->currFunc++;
	scrVM.currThread->scriptPos += jumpVal;
}

void op_popParams()
{
	char paramCount = scr_readChar();

	if (paramCount > SCR_MAX_VARS)
	{
		scr_error("Script call exceeded SCR_MAX_VARS parameters", false);
		return;
	}

	for (int i = 0; i < paramCount; i++)
	{
		scrVM.currThread->currFunc->vars[i] = scr_popFromStack();
	}
}

void op_end()
{
	if (scrVM.currThread->currFunc == scrVM.currThread->functionStack)
	{
		scr_killCurrThread();
	}
	else
	{
		scrVM.currThread->currFunc--;
		scrVM.currThread->scriptPos = scrVM.currThread->currFunc->callReturnPos;
	}
}

void op_wait()
{
	scrVM.currThread->isThreadPaused = true;
	scrVM.currThread->pauseStart = time_Milliseconds();
	scrVM.currThread->pauseTime = scr_popFloat();
}

void op_getVar()
{
	s_scriptVar var = scr_getVar(scr_readInteger());
	scr_pushToStack(var);
}

void op_setVar()
{
	s_scriptVar var = scr_popVar();
	scr_setVar(scr_readInteger(), var);
}

void op_add()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_add: var types aren't equal!", false);
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int + varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_float = varOne.val.t_float + varTwo.val.t_float;
		break;

	default:
		scr_error("op_add: var types won't add!", false);
		return;
	}
	
	scr_pushToStack(value);
}

void op_minus()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_minus: var types aren't equal!", false);
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int - varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_float = varOne.val.t_float - varTwo.val.t_float;
		break;

	default:
		scr_error("op_minus: var types won't minus!", false);
		return;
	}

	scr_pushToStack(value);
}

void op_multiply()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_multiply: var types aren't equal!", false);
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int * varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_float = varOne.val.t_float * varTwo.val.t_float;
		break;

	default:
		scr_error("op_multiply: var types won't multiply!", false);
		return;
	}

	scr_pushToStack(value);
}

void op_divide()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_divide: var types aren't equal!", false);
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int / varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_float = varOne.val.t_float / varTwo.val.t_float;
		break;

	default:
		scr_error("op_divide: var types won't divide!", false);
		return;
	}

	scr_pushToStack(value);
}

void initScriptVM()
{
	// Nothing needed
}

void addNewThread(char* scriptPos)
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
			
			return;
		}
	}

	error_noexit("Unable to exec new thread, more than %i threads executing currently\n", SCR_MAX_THREADS);
}

struct scriptHeader
{
	int scrLen;
	int mainPtr;
	char* scrStart;
};

int getBuiltinIndexFromHash(unsigned int hash)
{
	for (int i = 0; i < SCR_FUNC_COUNT; i++)
	{
		if (scrFuncArray[i].hash == hash)
			return i;
	}

	return 0;
}

bool linkScript(XScript* script)
{
	char* scrEnd = script->script + script->scrLen;
	char* scrPos = script->script; // skip scrlen and main func pointer

	int index;
	while (scrPos < scrEnd)
	{
		char opcode = *scrPos++; // make scrPos point to next value instead of curr opcode
		switch (opcode)
		{
		case 0x00: //op_pushInt
		case 0x01: //op_pushFloat
		case 0x02: //op_Jump
		case 0x06: //op_getVar
		case 0x07: //op_setVar
			scrPos += 4;
			break;

		case 0x03: //op_callBuiltin
			index = getBuiltinIndexFromHash(*(unsigned int*)scrPos);

			if (index == 0)
			{
				printf("linkScript hash %X not found at 0x%llX!", *(unsigned int*)scrPos, scrPos - script->script);
				return false;
			}

			*(unsigned int*)scrPos = index;
			scrPos += 4;
			break;

		case 0x04: //op_callScript
			scrPos += 4; // jump pos
			break;

		case 0x05: //op_wait
		case 0x08: //op_add
		case 0x09: //op_minus
		case 0x0A: //op_multiply
		case 0x0B: //op_divide
		case 0x0E: //op_end
			break;

		case 0x0D: //op_popParams
			scrPos++;
			break;

		case 0x0C: //op_pushString
			scrPos += strlen(scrPos) + 1;
			break;
		}
	}

	return true;
}

void executeScript(const char* scrName)
{
	XScript* script = findAsset(XASSET_SCRIPT, scrName).Script;

	if (script == NULL)
	{
		printf("cant find script %s\n", scrName);
		return;
	}

	if (!linkScript(script))
		return;

	// execute main func
	addNewThread(script->script + script->mainFuncPtr);
}

void (*opcodeFuncArray[])();

void execCurrThread()
{
	if(scrVM.currThread == NULL)
	{
		printf("scrVM.currThread was null!\n");
		return;
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
		unsigned char opcode = *scrVM.currThread->scriptPos;
		scrVM.currThread->scriptPos++;

		opcodeFuncArray[opcode]();
	}
}

void scr_runCurrentThreads()
{
	for (int i = 0; i < SCR_MAX_THREADS; i++)
	{
		if (scrVM.threads[i].isThreadActive)
		{
			scrVM.currThreadNum = i;
			scrVM.currThread = &scrVM.threads[i];
			execCurrThread();
		}
	}
}

void (*opcodeFuncArray[])() =
{
	op_pushInt,
	op_pushFloat,
	op_jump,
	op_callBuiltin,
	op_callScript,
	op_wait,
	op_getVar,
	op_setVar,
	op_add,
	op_minus,
	op_multiply,
	op_divide,
	op_pushString,
	op_popParams,
	op_end
};