#include "src/assetDB/AssetDB.h"

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

struct scriptVM
{
	char* scriptPos;

	s_scriptStack* currStack;
	s_scriptStack* maxStack;

	s_scriptStack stack[SCR_MAX_STACK];
};

scriptVM scrVM;

int scr_readInteger()
{
	int val = *(int*)scrVM.scriptPos;
	scrVM.scriptPos += sizeof(int);
	return val;
}

float scr_readFloat()
{
	float val = *(float*)scrVM.scriptPos;
	scrVM.scriptPos += sizeof(float);
	return val;
}

void op_pushFloat()
{
	scrVM.currStack++;

	scrVM.currStack->type = SCRIPT_FLOAT;
	scrVM.currStack->val.t_float = scr_readFloat();
}

void op_pushInteger()
{
	scrVM.currStack++;

	scrVM.currStack->type = SCRIPT_INTEGER;
	scrVM.currStack->val.t_int = scr_readInteger();
}

void op_jump()
{
	scrVM.scriptPos += scr_readInteger();
}

void op_call()
{

}

void (*opcodeFuncArray[])() =
{
	op_pushInteger,
	op_pushFloat,
	op_jump,
	op_call
};

struct scriptFuncDef
{
	void (*scriptFunc)();
	int id;
};

scriptFuncDef scriptFuncArray[] = 
{
	{op_call, 1}
};

void initScriptVM()
{
	scrVM.currStack = scrVM.stack;
	scrVM.maxStack = &scrVM.stack[SCR_MAX_STACK - 1];
}