#pragma once

struct scriptFunc
{
	const unsigned int hash;
	void (*func)();
};

extern scriptFunc scrFuncArray[2];
#define SCR_FUNC_COUNT sizeof(scrFuncArray)/sizeof(scrFuncArray[0])