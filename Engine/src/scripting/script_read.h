#pragma once

extern void (*opcodeFuncArray[])();

enum opcodeNameEnum
{
	e_op_voidOpcode,
	e_op_pushNull,
	e_op_pushBool,
	e_op_pushInt,
	e_op_pushFloat,
	e_op_pushString,
	e_op_jump,
	e_op_jumpOnTrue,
	e_op_jumpOnFalse,
	e_op_callBuiltin,
	e_op_callScript,
	e_op_callScriptThreaded,
	e_op_popParams,
	e_op_end,
	e_op_wait,
	e_op_getVar,
	e_op_setVar,
	e_op_or,
	e_op_xor,
	e_op_and,
	e_op_shiftLeft,
	e_op_shiftRight,
	e_op_add,                 // Var on top of stack is added to the var next on the stack
	e_op_minus,               // Var next on the stack is subtracted from the var on top of the stack
	e_op_multiply,            // Var on top of stack is multiplied with the var next on the stack
	e_op_divide,              // Var on top of stack is divided by the var next on the stack
	e_op_modulus,             // Var on top of stack is modulo by the var next on the stack
	e_op_equal,
	e_op_notEqual,
	e_op_lessThan,
	e_op_greaterThan,
	e_op_lessThanOrEqual,
	e_op_greaterThanOrEqual,

	e_opcodeMax
};