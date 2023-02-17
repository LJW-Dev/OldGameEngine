#pragma once

enum e_dvarTypes
{
	DVAR_INTEGER,
	DVAR_FLOAT,
	DVAR_BOOL
};

union u_dvarType
{
	int integer_t;
	float float_t;
	bool bool_t;
};

struct s_dvar
{
	const char* name;
	const char* description;
	e_dvarTypes type;

	u_dvarType curValue;
	u_dvarType minValue;
	u_dvarType maxValue;
};

s_dvar* dvar_RegisterInteger(const char* dvarName, const char* description, int defaultValue, int minValue, int maxValue);
s_dvar* dvar_RegisterFloat(const char* dvarName, const char* description, float defaultValue, float minValue, float maxValue);
s_dvar* dvar_RegisterBool(const char* dvarName, const char* description, bool defaultValue);

bool dvar_getBool(s_dvar* dvar);
int dvar_getInt(s_dvar* dvar);
float dvar_getFloat(s_dvar* dvar);

void dvar_setBool(s_dvar* dvar, bool value);
void dvar_setInt(s_dvar* dvar, int value);
void dvar_setFloat(s_dvar* dvar, float value);

s_dvar* dvar_findVar(char* dvarName);
e_dvarTypes dvar_getType(s_dvar* dvar);

void initDvars();

extern s_dvar* sv_overrideTimestep;
extern s_dvar* sv_timestep;
extern s_dvar* game_exit;
extern s_dvar* cl_wireframe;
extern s_dvar* cl_sensitivity;
