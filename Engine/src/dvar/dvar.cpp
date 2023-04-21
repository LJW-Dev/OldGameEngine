#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dvar.h"

#include "src/error/error.h"

#define DVAR_MAX 100

s_dvar dvarArray[DVAR_MAX];
int dvarCount = 0;

s_dvar* dvar_register(const char* dvarName, const char* description, e_dvarTypes type, u_dvarData dvarData)
{
	if (dvarCount >= DVAR_MAX)
	{
		error_exit("More than %i dvars!\n", DVAR_MAX);
	}

	if (dvarName == NULL)
	{
		error_exit("dvar name can't be null!\n");
	}

	for (int i = 0; dvarName[i] != '\0'; i++)
	{
		if (isalpha(dvarName[i]) && !islower(dvarName[i]))
		{
			error_exit("dvar %s must be all lowercase!\n", dvarName);
		}
	}

	s_dvar* currDvar = &dvarArray[dvarCount];
	dvarCount++;

	currDvar->name = dvarName;
	currDvar->description = description;
	currDvar->type = type;

	if (type == DVAR_FUNC)
	{
		currDvar->dvarData.function = dvarData.function;
	}
	else
	{
		currDvar->dvarData.data.curValue = dvarData.data.curValue;
		currDvar->dvarData.data.minValue = dvarData.data.minValue;
		currDvar->dvarData.data.maxValue = dvarData.data.maxValue;
	}

	return currDvar;
}

s_dvar* dvar_RegisterInteger(const char* dvarName, const char* description, int defaultValue, int minValue, int maxValue)
{
	u_dvarData dvarData;

	dvarData.data.minValue.integer_t = minValue;
	dvarData.data.maxValue.integer_t = maxValue;
	dvarData.data.curValue.integer_t = defaultValue;

	return dvar_register(dvarName, description, DVAR_INTEGER, dvarData);
}

s_dvar* dvar_RegisterFloat(const char* dvarName, const char* description, float defaultValue, float minValue, float maxValue)
{
	u_dvarData dvarData;

	dvarData.data.minValue.float_t = minValue;
	dvarData.data.maxValue.float_t = maxValue;
	dvarData.data.curValue.float_t = defaultValue;

	return dvar_register(dvarName, description, DVAR_FLOAT, dvarData);
}

s_dvar* dvar_RegisterBool(const char* dvarName, const char* description, bool defaultValue)
{
	u_dvarData dvarData;

	dvarData.data.minValue.bool_t = 0;
	dvarData.data.maxValue.bool_t = 1;
	dvarData.data.curValue.bool_t = defaultValue;

	return dvar_register(dvarName, description, DVAR_BOOL, dvarData);
}

s_dvar* dvar_RegisterFunction(const char* dvarName, const char* description, void (*function)())
{
	u_dvarData dvarData;

	dvarData.function = function;

	return dvar_register(dvarName, description, DVAR_FUNC, dvarData);
}

s_dvar* dvar_findVar(char* dvarName)
{
	for (int i = 0; i < dvarCount; i++)
	{
		s_dvar* currDvar = &dvarArray[i];

		if (currDvar->name != NULL)
		{
			if (strcmp(dvarName, currDvar->name) == 0)
				return currDvar;
		}
	}

	return NULL;
}

void dvar_call(s_dvar* dvar)
{
	if (dvar->type != DVAR_FUNC)
		error_exit("Dvar %s not a function!\n", dvar->name);

	dvar->dvarData.function();
}

bool dvar_getBool(s_dvar* dvar)
{
	if (dvar->type != DVAR_BOOL)
		error_exit("Dvar %s not a bool!\n", dvar->name);

	return dvar->dvarData.data.curValue.bool_t;
}

int dvar_getInt(s_dvar* dvar)
{
	if (dvar->type != DVAR_INTEGER)
		error_exit("Dvar %s not an integer!\n", dvar->name);

	return dvar->dvarData.data.curValue.integer_t;
}

float dvar_getFloat(s_dvar* dvar)
{
	if (dvar->type != DVAR_FLOAT)
		error_exit("Dvar %s not a float!\n", dvar->name);

	return dvar->dvarData.data.curValue.float_t;
}

e_dvarTypes dvar_getType(s_dvar* dvar)
{
	return dvar->type;
}

void dvar_setBool(s_dvar* dvar, bool value)
{
	if (dvar->type != DVAR_BOOL)
		error_exit("Dvar %s not a bool!\n", dvar->name);

	dvar->dvarData.data.curValue.bool_t = value;
}

void dvar_setInt(s_dvar* dvar, int value)
{
	if (dvar->type != DVAR_INTEGER)
		error_exit("Dvar %s not an integer!\n", dvar->name);

	if (value > dvar->dvarData.data.maxValue.integer_t)
	{
		value = dvar->dvarData.data.maxValue.integer_t;
		return;
	}
		
	if (value < dvar->dvarData.data.minValue.integer_t)
	{
		value = dvar->dvarData.data.minValue.integer_t;
		return;
	}

	dvar->dvarData.data.curValue.integer_t = value;
}

void dvar_setFloat(s_dvar* dvar, float value)
{
	if (dvar->type != DVAR_FLOAT)
		error_exit("Dvar %s not a float!\n", dvar->name);

	if (value > dvar->dvarData.data.maxValue.float_t)
	{
		value = dvar->dvarData.data.maxValue.float_t;
		return;
	}

	if (value < dvar->dvarData.data.minValue.float_t)
	{
		value = dvar->dvarData.data.minValue.float_t;
		return;
	}

	dvar->dvarData.data.curValue.float_t = value;
}