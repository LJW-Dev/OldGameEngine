#include "src/entity/Entity.h"
#include "script_func.h"
#include "script.h"

#include <stdio.h>

void spawnCube()
{
	float x = scr_popFloat();
	float y = scr_popFloat();
	float z = scr_popFloat();

	addEntity(x, y, z);
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
	}
}

unsigned int hashStr(const char* str) // djb2 hashing algorithm http://www.cse.yorku.ca/~oz/hash.html
{
	unsigned long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c;

	return (unsigned int)(hash & 0xFFFFFFFF);
}

scriptFunc scrFuncArray[2] =
{
	{hashStr("spawnCube"), spawnCube},
	{hashStr("debugPrint"), debugPrint},
};