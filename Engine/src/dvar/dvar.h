#pragma once

enum e_dvarTypes
{
	DVAR_INTEGER,
	DVAR_FLOAT,
	DVAR_BOOL,
	DVAR_FUNC
};

union u_dvarType
{
	int integer_t;
	float float_t;
	bool bool_t;
};

union u_dvarData
{
	struct s_dvarData
	{
		u_dvarType curValue;
		u_dvarType minValue;
		u_dvarType maxValue;
	} data;

	void (*function)();
};

struct s_dvar
{
	const char* name;
	const char* description;
	e_dvarTypes type;

	u_dvarData dvarData;
};

s_dvar* dvar_RegisterInteger(const char* dvarName, const char* description, int defaultValue, int minValue, int maxValue);
s_dvar* dvar_RegisterFloat(const char* dvarName, const char* description, float defaultValue, float minValue, float maxValue);
s_dvar* dvar_RegisterBool(const char* dvarName, const char* description, bool defaultValue);
s_dvar* dvar_RegisterFunction(const char* dvarName, const char* description, void (*function)());

bool dvar_getBool(s_dvar* dvar);
int dvar_getInt(s_dvar* dvar);
float dvar_getFloat(s_dvar* dvar);

void dvar_setBool(s_dvar* dvar, bool value);
void dvar_setInt(s_dvar* dvar, int value);
void dvar_setFloat(s_dvar* dvar, float value);
void dvar_call(s_dvar* dvar);

s_dvar* dvar_findVar(char* dvarName);
e_dvarTypes dvar_getType(s_dvar* dvar);