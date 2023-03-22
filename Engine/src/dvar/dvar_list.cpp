#include "dvar_list.h"
#include "src/window/Window.h"
#include "src/commands/commands.h"
#include "src/openGL/OpenGL.h"

#include <stdlib.h>

s_dvar* game_exit;

s_dvar* sv_overrideTimestep;
s_dvar* sv_timestep;
s_dvar* cl_wireframe;
s_dvar* cl_sensitivity;

void dvar_game_exit(s_paramInfo* arguments)
{
	window_windowShouldClose();
}

void dvar_cl_wireframe(s_paramInfo* arguments)
{
	char* argVal = command_getNextParam(arguments);
	if (argVal == NULL)
		return;

	setWireframe(atoi(argVal));
}

void initDvars()
{
	sv_overrideTimestep = dvar_RegisterBool("sv_overridetimestep", "Override frame based timestep", false);
	sv_timestep = dvar_RegisterFloat("sv_timestep", "Timestep when sv_overrideTimestep is true", 1.0f / 30.0f, 0, 1.0f);
	cl_sensitivity = dvar_RegisterFloat("cl_sensitivity", "Player sensitivity", 0.5f, 0.0f, 10.0f);

	cl_wireframe = dvar_RegisterFunction("cl_wireframe", "Enable wireframe", dvar_cl_wireframe);
	game_exit = dvar_RegisterFunction("game_exit", "Game will close when called", dvar_game_exit);
}