#include "commands.h"
#include "src/dvar/dvar.h"

#include <string.h>
#include <stdlib.h>

struct s_commandInfo
{
	char commandBuffer[1000];

	int argCount;

	int currArgNum;
	char* currArg;
};

s_commandInfo command;

char* command_getNextParam()
{
	if (command.currArgNum == command.argCount)
		return NULL;


	char* retVal = command.currArg;
	
	command.currArgNum++;
	command.currArg += strlen(command.currArg) + 1;

	return retVal;
}

void command_parse(const char* commandStr)
{
	strcpy_s(command.commandBuffer, commandStr);
	command.currArg = command.commandBuffer;
	command.currArgNum = 0;
	command.argCount = 1;

	int commandBufPos = 0;
	while (command.commandBuffer[commandBufPos] != 0)
	{
		if (command.commandBuffer[commandBufPos] == ' ')
		{
			command.commandBuffer[commandBufPos] = '\0';
			command.argCount++;
		}

		commandBufPos++;
	}
}

void command_send(const char* command)
{
	command_parse(command);
	
	s_dvar* dvar = dvar_findVar(command_getNextParam());

	if (dvar == NULL)
		return;

	switch (dvar_getType(dvar))
	{
	case DVAR_BOOL:
		dvar_setBool(dvar, atol(command_getNextParam()) != 0);
		break;

	case DVAR_FLOAT:
		dvar_setFloat(dvar, atof(command_getNextParam()));
		break;

	case DVAR_INTEGER:
		dvar_setInt(dvar, atol(command_getNextParam()));
		break;

	case DVAR_FUNC:
		dvar_call(dvar);
		break;
	}
}