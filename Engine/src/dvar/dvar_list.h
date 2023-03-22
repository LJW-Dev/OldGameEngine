#pragma once

#include "dvar.h"

extern s_dvar* sv_overrideTimestep;
extern s_dvar* sv_timestep;
extern s_dvar* game_exit;
extern s_dvar* cl_wireframe;
extern s_dvar* cl_sensitivity;

void initDvars();