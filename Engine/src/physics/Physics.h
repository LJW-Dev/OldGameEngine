#pragma once

#include <glm/glm.hpp>
using namespace glm;

#define phys_gravity 2.0f
#define phys_friction 20.0f

#define phys_stopspeed 1.0f

#define phys_acceleration 5.0f

#define phys_jumpForce 0.5f

#include "src/player/Player.h"

void updatePlayerPhysics(double deltaTime);
void initWorldPhysics();