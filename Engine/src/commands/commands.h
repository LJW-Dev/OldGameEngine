#pragma once

struct s_paramInfo
{
	int argCount;

	int currArgNum;
	char* currArg;
};

void command_send(const char* command);
char* command_getNextParam(s_paramInfo* paramInfo);