#include <stdio.h>
#include <stdarg.h>

#include "error.h"

jmp_buf mainErrorBuf;

#define ERROR_STATUS -1

[[noreturn]] void error_exit(const char* format, ...)
{
	char buffer[1000];

	va_list args;
	va_start(args, format);

	vsnprintf(buffer, 256, format, args);
	printf(buffer);

	va_end(args);

	longjmp(mainErrorBuf, ERROR_STATUS);
}

void error_noexit(const char* format, ...)
{
	char buffer[1000];

	va_list args;
	va_start(args, format);

	vsnprintf(buffer, 256, format, args);
	printf(buffer);

	va_end(args);
}