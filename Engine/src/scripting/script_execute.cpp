#include "src/assetDB/AssetDB.h"

#include <GLFW/glfw3.h>

#include "src/OpenGL.h"

enum e_scriptVarType
{
	SCRIPT_INTEGER,
	SCRIPT_FLOAT
};

union u_scriptVarVal
{
	int t_int;
	float t_float;
};

struct s_scriptStack
{
	e_scriptVarType type;
	u_scriptVarVal val;
};

#define SCR_MAX_STACK 100

struct scrThreadInfo
{
	bool isThreadActive;
	bool isThreadPaused;

	char* scriptPos;

	s_scriptStack* freeStack;
	s_scriptStack* maxStack;

	s_scriptStack stack[SCR_MAX_STACK]; 
	
	double waitStart;
	double waitTime;
};

#define SCR_MAX_THREADS 10

struct scriptVM
{
	scrThreadInfo* currThread;
	
	scrThreadInfo threads[SCR_MAX_THREADS];
};

scriptVM scrVM;

void (*scriptFuncArray[])();
void (*opcodeFuncArray[])();

int scr_getInteger()
{
	scrVM.currThread->freeStack--;
	return scrVM.currThread->freeStack->val.t_int;
}

float scr_getFloat()
{
	scrVM.currThread->freeStack--;
	return scrVM.currThread->freeStack->val.t_float;

}

int scr_peekInteger()
{
	return *(int*)scrVM.currThread->scriptPos;
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

void op_pushFloat()
{
	if (scrVM.currThread->freeStack > scrVM.currThread->maxStack)
	{
		printf("Max stack reached!");
		return;
	}

	scrVM.currThread->freeStack->type = SCRIPT_FLOAT;
	scrVM.currThread->freeStack->val.t_float = scr_readFloat();

	scrVM.currThread->freeStack++;
}

void op_pushInteger()
{
	if (scrVM.currThread->freeStack > scrVM.currThread->maxStack)
	{
		printf("Max stack reached!");
		return;
	}

	scrVM.currThread->freeStack->type = SCRIPT_INTEGER;
	scrVM.currThread->freeStack->val.t_int = scr_readInteger();

	scrVM.currThread->freeStack++;
}

void op_jump()
{
	scrVM.currThread->scriptPos += scr_peekInteger();
}

void op_call()
{
	scriptFuncArray[scr_readInteger()]();
}

void op_end()
{
	scrVM.currThread->isThreadActive = false;
}

void op_wait()
{
	scrVM.currThread->isThreadPaused = true;
	scrVM.currThread->waitStart = glfwGetTime();
	scrVM.currThread->waitTime = scr_readFloat();
}

void spawnCube()
{
	float x = scr_getFloat();
	float y = scr_getFloat();
	float z = scr_getFloat();

	addRenderObject(x, y, z);
}

void initScriptVM()
{
	scrVM.currThread = &scrVM.threads[0];
}

void executeScript(const char* scrName)
{
	XScript* script = findAsset(XASSET_SCRIPT, scrName).Script;

	if (script == NULL)
	{
		printf("cant find script %s\n", scrName);
		return;
	}

	if (scrVM.currThread > &scrVM.threads[SCR_MAX_THREADS - 1])
	{
		printf("Unable to exec thread, more than %i threads executing currently\n", SCR_MAX_THREADS);
		return;
	}

	for (int i = 0; i < SCR_MAX_THREADS; i++)
	{
		if (!scrVM.threads[i].isThreadActive)
		{
			scrVM.threads[i].scriptPos = script->script;
			scrVM.threads[i].isThreadActive = true;
			scrVM.threads[i].isThreadPaused = false;
			scrVM.threads[i].freeStack = scrVM.currThread->stack;
			scrVM.threads[i].maxStack = &scrVM.currThread->stack[SCR_MAX_STACK - 1];

			break;
		}
	}
}

void execCurrThread()
{
	if (scrVM.currThread->isThreadPaused)
	{
		if (scrVM.currThread->waitStart + scrVM.currThread->waitTime > glfwGetTime())
			return;
		else
			scrVM.currThread->isThreadPaused = false;
	}

	while (!scrVM.currThread->isThreadPaused && scrVM.currThread->isThreadActive)
	{
		char opcode = *scrVM.currThread->scriptPos;
		scrVM.currThread->scriptPos++;

		opcodeFuncArray[opcode]();
	}
}

void runCurrentThreads()
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
	spawnCube
};

void (*opcodeFuncArray[])() =
{
	op_pushInteger,
	op_pushFloat,
	op_jump,
	op_call,
	op_wait,
	op_end
};