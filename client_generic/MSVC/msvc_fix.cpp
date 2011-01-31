#include "msvc_fix.h"
#include <windows.h>
#include <stdio.h> 
#include <stdarg.h>

int snprintf(char *buffer, size_t count, const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = _vsnprintf(buffer, count-1, fmt, ap);
	if (ret < 0)
		buffer[count-1] = '\0';
	va_end(ap);
	return ret;
}