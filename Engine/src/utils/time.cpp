#include <GLFW/glfw3.h>

#include "time.h"

bool isFirstCall;
double firstCallTime;

double time_Milliseconds()
{
	return glfwGetTime();
}

/*
double time_Milliseconds()
{
	if (!isFirstCall)
	{
		firstCallTime = glfwGetTime();
		isFirstCall = true;
	}

	return glfwGetTime() - firstCallTime;
}
*/