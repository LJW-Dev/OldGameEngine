#include "src/assetDB/AssetDB.h"

#include <GLFW/glfw3.h>

#include "src/OpenGL.h"

#include <string.h>

enum e_scriptVarType
{
	SCRIPT_INTEGER,
	SCRIPT_FLOAT,
	SCRIPT_STRING
};

union u_scriptVarVal
{
	char* t_str;
	int t_int;
	float t_float;
};

struct s_scriptVar
{
	e_scriptVarType type;
	u_scriptVarVal val;
};

#define SCR_MAX_VARS 20

struct localFunction
{
	char* funcStart;

	char* subCallJumpBack;

	s_scriptVar vars[SCR_MAX_VARS];
};

#define SCR_MAX_STACK 10
#define SCR_MAX_CALLS 20

struct scrThreadInfo
{
	bool isThreadActive;
	bool isThreadPaused;

	char* scriptPos;

	s_scriptVar* freeStack;
	s_scriptVar* maxStack;
	s_scriptVar stack[SCR_MAX_STACK];
	
	double waitStart;
	double waitTime;

	localFunction* currFunc;
	localFunction* maxFunc;
	localFunction functionStack[SCR_MAX_CALLS];
};

#define SCR_MAX_THREADS 10

struct scriptVM
{
	scrThreadInfo* currThread;
	scrThreadInfo* maxThread;
	scrThreadInfo threads[SCR_MAX_THREADS];
};

scriptVM scrVM;

void (*scriptFuncArray[])();
void (*opcodeFuncArray[])();

void scr_killCurrThread()
{
	printf("Thread killed");
	scrVM.currThread->isThreadActive = false;
}

void scr_error(const char* msg)
{
	printf("scr_error: %s", msg);
	scr_killCurrThread();
}

void scr_pushToStack(s_scriptVar data)
{
	if (scrVM.currThread->freeStack > scrVM.currThread->maxStack)
	{
		scr_error("Max stack reached!");
		return;
	}

	scrVM.currThread->freeStack->type = data.type;
	scrVM.currThread->freeStack->val = data.val;

	scrVM.currThread->freeStack++;
}

s_scriptVar scr_popFromStack()
{
	if (scrVM.currThread->freeStack < scrVM.currThread->stack)
	{
		scr_error("Min stack reached!");
		return *scrVM.currThread->stack;
	}

	scrVM.currThread->freeStack--;
	return *scrVM.currThread->freeStack;
}

void scr_addVarsFromStack(int count)
{
	for (int i = 0; i < count; i++)
	{
		scrVM.currThread->currFunc->vars[i] = scr_popFromStack();
	}
}

s_scriptVar scr_getVar()
{
	return scr_popFromStack();
}

int scr_getInteger()
{
	return scr_popFromStack().val.t_int;
}

float scr_getFloat()
{
	return scr_popFromStack().val.t_float;
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
	scriptFuncArray[scr_readInteger()]();
}

void op_callScript()
{
	if (scrVM.currThread->currFunc == scrVM.currThread->maxFunc)
	{
		scr_error("Too many subcalls in one thread!");
		return;
	}
	char paramCount = scr_readChar();
	int jumpVal = scr_peekInteger();

	scrVM.currThread->currFunc->subCallJumpBack = scrVM.currThread->scriptPos + sizeof(int);
	
	scrVM.currThread->currFunc++;
	scrVM.currThread->currFunc->funcStart = scrVM.currThread->scriptPos + jumpVal;
	scrVM.currThread->scriptPos += jumpVal;

	scr_addVarsFromStack(paramCount);
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
		scrVM.currThread->scriptPos = scrVM.currThread->currFunc->subCallJumpBack;
	}
}

void op_wait()
{
	scrVM.currThread->isThreadPaused = true;
	scrVM.currThread->waitStart = glfwGetTime();
	scrVM.currThread->waitTime = scr_getFloat();
}

void op_getVar()
{
	s_scriptVar var = scrVM.currThread->currFunc->vars[scr_readInteger()];

	scr_pushToStack(var);
}

void op_setVar()
{
	s_scriptVar* baseVar = &scrVM.currThread->currFunc->vars[scr_readInteger()];
	s_scriptVar newVar = scr_getVar();
	
	baseVar->type = newVar.type;
	baseVar->val = newVar.val;
}

void op_add()
{
	s_scriptVar varOne = scr_getVar();
	s_scriptVar varTwo = scr_getVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_add wrong params!");
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
		scr_error("incorrect divide type!");
		return;
	}
	
	scr_pushToStack(value);
}

void op_minus()
{
	s_scriptVar varOne = scr_getVar();
	s_scriptVar varTwo = scr_getVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_minus wrong params!");
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
		scr_error("incorrect divide type!");
		return;
	}

	scr_pushToStack(value);
}

void op_multiply()
{
	s_scriptVar varOne = scr_getVar();
	s_scriptVar varTwo = scr_getVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_multiply wrong params!");
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
		scr_error("incorrect divide type!");
		return;
	}

	scr_pushToStack(value);
}

void op_divide()
{
	s_scriptVar varOne = scr_getVar();
	s_scriptVar varTwo = scr_getVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_divide wrong params!");
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
		scr_error("incorrect divide type!");
		return;
	}

	scr_pushToStack(value);
}

void spawnCube()
{
	float x = scr_getFloat();
	float y = scr_getFloat();
	float z = scr_getFloat();

	addRenderObject(x, y, z);
}

void debugPrint()
{
	s_scriptVar var = scr_getVar();

	switch (var.type)
	{
	case SCRIPT_INTEGER:
		printf("%i\n", var.val.t_int);
		break;

	case SCRIPT_FLOAT:
		printf("%f\n", var.val.t_float);
		break;

	case SCRIPT_STRING:
		printf("%s\n", var.val.t_str);
		break;
	}
}

void initScriptVM()
{
	scrVM.maxThread = &scrVM.threads[SCR_MAX_THREADS - 1];
	scrVM.currThread = NULL;
}

void addNewThread(char* scriptPos)
{
	if (scrVM.currThread > scrVM.maxThread)
	{
		printf("Unable to exec thread, more than %i threads executing currently\n", SCR_MAX_THREADS);
		return;
	}

	for (int i = 0; i < SCR_MAX_THREADS; i++)
	{
		if (!scrVM.threads[i].isThreadActive)
		{
			scrVM.threads[i].isThreadActive = true;
			scrVM.threads[i].isThreadPaused = false;

			scrVM.threads[i].scriptPos = scriptPos;
			
			scrVM.threads[i].freeStack = scrVM.threads[i].stack;
			scrVM.threads[i].maxStack = &scrVM.threads[i].stack[SCR_MAX_STACK - 1];

			scrVM.threads[i].waitStart = -1.0f;
			scrVM.threads[i].waitTime = -1.0f;

			scrVM.threads[i].maxFunc = &scrVM.threads[i].functionStack[SCR_MAX_CALLS - 1];
			scrVM.threads[i].currFunc = scrVM.threads[i].functionStack;
			scrVM.threads[i].currFunc->funcStart = scriptPos;

			break;
		}
	}
}

void executeScript(const char* scrName)
{
	XScript* script = findAsset(XASSET_SCRIPT, scrName).Script;

	if (script == NULL)
	{
		printf("cant find script %s\n", scrName);
		return;
	}

	// get main func start
	char* startPos = (*(int*)script->script) + script->script;
	addNewThread(startPos);
}

void execCurrThread()
{
	if(scrVM.currThread == NULL)
	{
		printf("scrVM.currThread was null!\n");
		return;
	}

	if (scrVM.currThread->isThreadPaused)
	{
		if (scrVM.currThread->waitStart + scrVM.currThread->waitTime > glfwGetTime())
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
			scrVM.currThread = &scrVM.threads[i];
			execCurrThread();
		}
	}
}

void (*scriptFuncArray[])() =
{
	spawnCube,
	debugPrint
};

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
	op_end
};