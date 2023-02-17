#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dvar.h"

#define DVAR_MAX 100

s_dvar dvarArray[DVAR_MAX];
int dvarCount = 0;

s_dvar* dvar_Register(const char* dvarName, const char* description, e_dvarTypes type, u_dvarType defaultValue, u_dvarType minValue, u_dvarType maxValue)
{
	if (dvarCount >= DVAR_MAX)
	{
		printf("dvar count exceeded!");
		return NULL;
	}

	if (dvarName == NULL)
	{
		printf("dvar name can't be null!");
		return NULL;
	}

	for (int i = 0; dvarName[i] != '\0'; i++)
	{
		if (isalpha(dvarName[i]) && !islower(dvarName[i]))
		{
			printf("dvar %s must be all lowercase!", dvarName);
			return NULL;
		}
	}

	s_dvar* currDvar = &dvarArray[dvarCount];

	currDvar->name = dvarName;
	currDvar->description = description;
	currDvar->type = type;
	currDvar->curValue = defaultValue;
	currDvar->minValue = minValue;
	currDvar->maxValue = maxValue;

	dvarCount++;

	return currDvar;
}

s_dvar* dvar_RegisterInteger(const char* dvarName, const char* description, int defaultValue, int minValue, int maxValue)
{
	u_dvarType minVal;
	u_dvarType maxVal;
	u_dvarType defValue;

	minVal.integer_t = minValue;
	maxVal.integer_t = maxValue;
	defValue.integer_t = defaultValue;

	return dvar_Register(dvarName, description, DVAR_INTEGER, defValue, minVal, maxVal);
}

s_dvar* dvar_RegisterFloat(const char* dvarName, const char* description, float defaultValue, float minValue, float maxValue)
{
	u_dvarType minVal;
	u_dvarType maxVal;
	u_dvarType defValue;

	minVal.float_t = minValue;
	maxVal.float_t = maxValue;
	defValue.float_t = defaultValue;

	return dvar_Register(dvarName, description, DVAR_FLOAT, defValue, minVal, maxVal);
}

s_dvar* dvar_RegisterBool(const char* dvarName, const char* description, bool defaultValue)
{
	u_dvarType minVal;
	u_dvarType maxVal;
	u_dvarType defValue;

	minVal.bool_t = false;
	maxVal.bool_t = true;
	defValue.bool_t = defaultValue;

	return dvar_Register(dvarName, description, DVAR_BOOL, defValue, minVal, maxVal);
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

bool dvar_getBool(s_dvar* dvar)
{
	return dvar->curValue.bool_t;
}

int dvar_getInt(s_dvar* dvar)
{
	return dvar->curValue.integer_t;
}

float dvar_getFloat(s_dvar* dvar)
{
	return dvar->curValue.float_t;
}

e_dvarTypes dvar_getType(s_dvar* dvar)
{
	return dvar->type;
}

void dvar_setBool(s_dvar* dvar, bool value)
{
	if (dvar->type != DVAR_BOOL)
		return;

	dvar->curValue.bool_t = value;
}

void dvar_setInt(s_dvar* dvar, int value)
{
	if (dvar->type != DVAR_INTEGER)
		return;

	if (value > dvar->maxValue.integer_t)
	{
		value = dvar->maxValue.integer_t;
		return;
	}
		
	if (value < dvar->minValue.integer_t)
	{
		value = dvar->minValue.integer_t;
		return;
	}

	dvar->curValue.integer_t = value;
}

void dvar_setFloat(s_dvar* dvar, float value)
{
	if (dvar->type != DVAR_FLOAT)
		return;

	if (value > dvar->maxValue.float_t)
	{
		value = dvar->maxValue.float_t;
		return;
	}

	if (value < dvar->minValue.float_t)
	{
		value = dvar->minValue.float_t;
		return;
	}

	dvar->curValue.float_t = value;
}

s_dvar* sv_overrideTimestep;
s_dvar* sv_timestep;
s_dvar* game_exit;
s_dvar* cl_wireframe;
s_dvar* cl_sensitivity;

void initDvars()
{
	sv_overrideTimestep = dvar_RegisterBool("sv_overridetimestep", "Override frame based timestep", false);
	sv_timestep = dvar_RegisterFloat("sv_timestep", "Timestep when sv_overrideTimestep is true", 1.0f / 30.0f, 0, 1.0f);
	game_exit = dvar_RegisterBool("game_exit", "Game will close when true", false);
	cl_wireframe = dvar_RegisterBool("cl_wireframe", "Enable wireframe", false);
	cl_sensitivity = dvar_RegisterFloat("cl_sensitivity", "Player sensitivity", 0.5f, 0.0f, 10.0f);
}