#include "script_vm.h"
#include "script_func.h"
#include "src/error/error.h"
#include "src/utils/time.h"
#include <string>

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

int scr_peekInteger()
{
	return *(int*)scrVM.currThread->scriptPos;
}

__int64 scr_readLong()
{
	__int64 val = *(__int64*)scrVM.currThread->scriptPos;
	scrVM.currThread->scriptPos += sizeof(__int64);
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
	char* val = *(char**)scrVM.currThread->scriptPos;
	scrVM.currThread->scriptPos += sizeof(char*);
	return val;
}

void op_pushNull()
{
	s_scriptVar value;
	value.type = SCRIPT_NULL;
	value.val.t_data = NULL;

	scr_pushToStack(value);
}

void op_pushBool()
{
	s_scriptVar value;
	value.type = SCRIPT_BOOL;
	value.val.t_bool = scr_readChar();

	scr_pushToStack(value);
}

void op_pushInt()
{
	s_scriptVar value;
	value.type = SCRIPT_INTEGER;
	value.val.t_int = scr_readInteger();

	scr_pushToStack(value);
}

void op_pushFloat()
{
	s_scriptVar value;
	value.type = SCRIPT_FLOAT;
	value.val.t_float = scr_readFloat();

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

void op_jumpOnTrue()
{
	char isTrue = scr_popBool();

	if (isTrue)
		scrVM.currThread->scriptPos += scr_peekInteger();
	else
		scr_readInteger();
}

void op_jumpOnFalse()
{
	char isTrue = scr_popBool();

	if (!isTrue)
		scrVM.currThread->scriptPos += scr_peekInteger();
	else
		scr_readInteger();
}

void op_callBuiltin()
{
	__int64 functionPtr = scr_readLong();

	auto func = (void (*)())functionPtr;
	func();
}

void op_callScript()
{
	if (scrVM.currThread->currFunc == scrVM.currThread->maxFunc)
	{
		scr_error("op_callScript: Too many nested calls in one thread!\n");
		return;
	}

	char* jumpStart = scrVM.currThread->scriptPos;
	char paramCount = scr_readChar();
	char* jumpPos = jumpStart + scr_readInteger();

	scrVM.currThread->currFunc->callReturnPos = scrVM.currThread->scriptPos;

	scrVM.currThread->currFunc++;
	scrVM.currThread->scriptPos = jumpPos;

	popFunctionParams(paramCount, scrVM.currThreadIndex);
}

void op_callScriptThreaded()
{
	if (scrVM.currThread->currFunc == scrVM.currThread->maxFunc)
	{
		scr_error("op_callScriptThreaded: Too many nested calls in one thread!\n");
		return;
	}

	char* jumpStart = scrVM.currThread->scriptPos;

	char paramCount = scr_readChar();
	addNewThread(jumpStart + scr_readInteger(), paramCount);
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
		scr_error("op_add: var types aren't equal!");
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
		scr_error("op_add: var types can't add!");
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
		scr_error("op_minus: var types aren't equal!");
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
		scr_error("op_minus: var types can't minus!");
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
		scr_error("op_multiply: var types aren't equal!");
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
		scr_error("op_multiply: var types can't multiply!");
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
		scr_error("op_divide: var types aren't equal!");
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
		scr_error("op_divide: var types can't divide!");
		return;
	}

	scr_pushToStack(value);
}

void op_modulus()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_modulus: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int % varTwo.val.t_int;
		break;

	default:
		scr_error("op_modulus: var types can't modulo!");
		return;
	}

	scr_pushToStack(value);
}

void op_or()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_or: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int | varTwo.val.t_int;
		break;

	default:
		scr_error("op_or: var types can't OR!");
		return;
	}

	scr_pushToStack(value);
}

void op_xor()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_xor: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int ^ varTwo.val.t_int;
		break;

	default:
		scr_error("op_xor: var types can't XOR!");
		return;
	}

	scr_pushToStack(value);
}

void op_and()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_and: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int & varTwo.val.t_int;
		break;

	default:
		scr_error("op_and: var types can't AND!");
		return;
	}

	scr_pushToStack(value);
}

void op_shiftLeft()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_shiftLeft: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int << varTwo.val.t_int;
		break;

	default:
		scr_error("op_shiftLeft: var types can't leftshift!");
		return;
	}

	scr_pushToStack(value);
}

void op_shiftRight()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_shiftRight: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = varOne.type;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_int = varOne.val.t_int >> varTwo.val.t_int;
		break;

	default:
		scr_error("op_shiftRight: var types can't rightshift!");
		return;
	}

	scr_pushToStack(value);
}

void op_equal()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_equal: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = SCRIPT_BOOL;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_bool = varOne.val.t_int == varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_bool = abs(varOne.val.t_float - varTwo.val.t_float) < 0.000001f;
		break;

	case SCRIPT_BOOL:
		value.val.t_bool = varOne.val.t_bool == varTwo.val.t_bool;
		break;

	case SCRIPT_NULL:
		value.val.t_bool = true;
		break;

	case SCRIPT_STRING:
		value.val.t_bool = strcmp(varOne.val.t_str, varOne.val.t_str) == 0;
		break;

	default:
		scr_error("op_equal: var types can't be checked for equality!");
		return;
	}

	scr_pushToStack(value);
}

void op_notEqual()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_notEqual: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = SCRIPT_BOOL;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_bool = varOne.val.t_int != varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_bool = abs(varOne.val.t_float - varTwo.val.t_float) > 0.000001f;
		break;

	case SCRIPT_BOOL:
		value.val.t_bool = varOne.val.t_bool != varTwo.val.t_bool;
		break;

	case SCRIPT_NULL:
		value.val.t_bool = false;
		break;

	case SCRIPT_STRING:
		value.val.t_bool = strcmp(varOne.val.t_str, varOne.val.t_str) != 0;
		break;

	default:
		scr_error("op_notEqual: var types can't be checked for equality!");
		return;
	}

	scr_pushToStack(value);
}

void op_lessThan()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_lessThan: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = SCRIPT_BOOL;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_bool = varOne.val.t_int < varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_bool = varOne.val.t_float < varTwo.val.t_float;
		break;

	default:
		scr_error("op_lessThan: var types can't be checked for equality!");
		return;
	}

	scr_pushToStack(value);
}

void op_greaterThan()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_greaterThan: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = SCRIPT_BOOL;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_bool = varOne.val.t_int > varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_bool = varOne.val.t_float > varTwo.val.t_float;
		break;

	default:
		scr_error("op_greaterThan: var types can't be checked for equality!");
		return;
	}

	scr_pushToStack(value);
}

void op_lessThanOrEqual()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_lessThan: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = SCRIPT_BOOL;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_bool = varOne.val.t_int <= varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_bool = varOne.val.t_float <= varTwo.val.t_float;
		break;

	default:
		scr_error("op_lessThan: var types can't be checked for equality!");
		return;
	}

	scr_pushToStack(value);
}

void op_greaterThanOrEqual()
{
	s_scriptVar varOne = scr_popVar();
	s_scriptVar varTwo = scr_popVar();

	if (varOne.type != varTwo.type)
	{
		scr_error("op_greaterThanOrEqual: var types aren't equal!");
		return;
	}

	s_scriptVar value;
	value.type = SCRIPT_BOOL;

	switch (varOne.type)
	{
	case SCRIPT_INTEGER:
		value.val.t_bool = varOne.val.t_int >= varTwo.val.t_int;
		break;

	case SCRIPT_FLOAT:
		value.val.t_bool = varOne.val.t_float >= varTwo.val.t_float;
		break;

	default:
		scr_error("op_greaterThanOrEqual: var types can't be checked for equality!");
		return;
	}

	scr_pushToStack(value);
}

void op_voidOpcode()
{
	scr_error("op_voidOpcode was called.");
}

void (*opcodeFuncArray[])() =
{
	op_voidOpcode, // scripts are mostly 0s, so scr_error will be called if the opcode is 0
	op_pushNull,
	op_pushBool,
	op_pushInt,
	op_pushFloat,
	op_pushString,
	op_jump,
	op_jumpOnTrue,
	op_jumpOnFalse,
	op_callBuiltin,
	op_callScript,
	op_callScriptThreaded,
	op_end,
	op_wait,
	op_getVar,
	op_setVar,
	op_or,
	op_xor,
	op_and,
	op_shiftLeft,
	op_shiftRight,
	op_add,                 // Var on top of stack is added to the var next on the stack
	op_minus,               // Var next on the stack is subtracted from the var on top of the stack
	op_multiply,            // Var on top of stack is multiplied with the var next on the stack
	op_divide,              // Var on top of stack is divided by the var next on the stack
	op_modulus,             // Var on top of stack is modulo by the var next on the stack
	op_equal,
	op_notEqual,
	op_lessThan,
	op_greaterThan,
	op_lessThanOrEqual,
	op_greaterThanOrEqual
};