#pragma once

enum e_scriptVarType
{
	SCRIPT_INTEGER,
	SCRIPT_FLOAT,
	SCRIPT_STRING
};

union u_scriptVarVal
{
	char* t_str;
	int t_int;
	float t_float;
};

struct s_scriptVar
{
	e_scriptVarType type;
	u_scriptVarVal val;
};

void scr_runCurrentThreads();
void executeScript(const char* scrName);
void initScriptVM();

s_scriptVar scr_popVar();
int scr_popInteger();
float scr_popFloat();