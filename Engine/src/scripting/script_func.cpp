#include "src/entity/Entity.h"
#include "script_func.h"
#include "script.h"
#include <string>

#include <stdio.h>

void moveEntity()
{
	s_scriptVar entVar = scr_popVar();
	if (entVar.type != SCRIPT_DATA)
	{
		scr_error("rotateEntity: not an entity!");
		return;
	}

	entityInfo* ent = (entityInfo*)entVar.val.t_data;

	ent->xPos = scr_popFloat();
	ent->yPos = scr_popFloat();
	ent->zPos = scr_popFloat();
}

void spawnModel()
{
	char* model = scr_popString();
	float x = scr_popFloat();
	float y = scr_popFloat();
	float z = scr_popFloat();

	entityInfo* ent = addEntity(x, y, z, model);
	if (ent == NULL)
		scr_error("Cannot spawn entity.");

	s_scriptVar var;
	var.type = SCRIPT_DATA;
	var.val.t_data = ent;

	scr_pushVar(var);
}

void debugPrint()
{
	s_scriptVar var = scr_popVar();

	switch (var.type)
	{
	case SCRIPT_INTEGER:
		printf("%i\n", var.val.t_int);
		break;

	case SCRIPT_FLOAT:
		printf("%f\n", var.val.t_float);
		break;

	case SCRIPT_STRING:
		printf("%s\n", var.val.t_str);
		break;

	case SCRIPT_BOOL:
		printf("%c\n", var.val.t_bool);
		break;

	case SCRIPT_NULL:
		printf("null\n");
		break;

	default:
		scr_error("debugPrint: wrong var type.");
		return;
	}
}

struct scriptFunc
{
	const char* name;
	scrFuncPtr func;
};

scriptFunc scrFuncArray[] =
{
	{"spawnModel", spawnModel},
	{"debugPrint", debugPrint},
	{"moveEntity", moveEntity}
};

#define SCR_FUNC_COUNT sizeof(scrFuncArray)/sizeof(scrFuncArray[0])

scrFuncPtr doesFunctionExist(char* name)
{
	for (int i = 0; i < SCR_FUNC_COUNT; i++)
	{
		if (strcmp(scrFuncArray[i].name, name) == 0)
			return scrFuncArray[i].func;
	}

	return NULL;
}