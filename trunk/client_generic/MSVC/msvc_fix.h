#pragma once
#include <direct.h>
#include <string>

int snprintf(char *buffer, size_t count, const char *fmt, ...);

//#ifdef _DEBUG
//#include <crtdbg.h>
//#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
//#else
//#define DEBUG_NEW new
//#endif