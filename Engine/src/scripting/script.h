#pragma once

enum e_scriptVarType
{
	SCRIPT_NULL,
	SCRIPT_BOOL,
	SCRIPT_INTEGER,
	SCRIPT_FLOAT,
	SCRIPT_STRING,
	SCRIPT_DATA
};

union u_scriptVarVal
{
	char t_bool;
	int t_int;
	float t_float;
	char* t_str;
	void* t_data;
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
char* scr_popString();
char scr_popBool();
void scr_error(const char* msg);

void scr_pushInteger(int value);
void scr_pushVar(s_scriptVar var);
void scr_pushFloat(float value);
void scr_pushString(char* value);