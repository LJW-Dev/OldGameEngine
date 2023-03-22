#include <string.h>
#include <stdlib.h>
#include "src/dvar/dvar.h"
#include "commands.h"

char command_buffer[1000];

char* command_getNextParam(s_paramInfo* paramInfo)
{
	char* retVal = paramInfo->currArg;

	if(paramInfo->currArgNum == paramInfo->argCount)
		return NULL;

	paramInfo->currArgNum++;
	paramInfo->currArg += strlen(paramInfo->currArg) + 1;

	return retVal;
}

void command_parse(const char* command, s_paramInfo* paramInfo)
{
	strcpy_s(command_buffer, command);
	paramInfo->currArg = command_buffer;
	paramInfo->currArgNum = 0;
	paramInfo->argCount = 1;

	int commandBufPos = 0;
	while (command_buffer[commandBufPos] != 0)
	{
		if (command_buffer[commandBufPos] == ' ')
		{
			command_buffer[commandBufPos] = '\0';
			paramInfo->argCount++;
		}

		commandBufPos++;
	}
}

void command_send(const char* command)
{
	s_paramInfo paramInfo;
	command_parse(command, &paramInfo);
	
	s_dvar* dvar = dvar_findVar(command_getNextParam(&paramInfo));

	if (dvar == NULL)
		return;

	switch (dvar_getType(dvar))
	{
	case DVAR_BOOL:
		dvar_setBool(dvar, atol(command_getNextParam(&paramInfo)) != 0);
		break;

	case DVAR_FLOAT:
		dvar_setFloat(dvar, atof(command_getNextParam(&paramInfo)));
		break;

	case DVAR_INTEGER:
		dvar_setInt(dvar, atol(command_getNextParam(&paramInfo)));
		break;

	case DVAR_FUNC:
		dvar_call(dvar, &paramInfo);
		break;
	}
}