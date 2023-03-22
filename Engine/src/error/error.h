#pragma once

#include <csetjmp>

extern jmp_buf mainErrorBuf;

void error_exit(const char* format, ...);
void error_noexit(const char* format, ...);