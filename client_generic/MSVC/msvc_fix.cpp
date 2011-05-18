#include "msvc_fix.h"
#include <windows.h>
#include <stdio.h> 
#include <stdarg.h>
#ifdef _WIN64
#include <winsock2.h>
#endif
extern "C"
{

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

#ifdef _WIN64
void ___chkstk() // remove after ffmpeg stops using too big arrays on stack
{
}

void usleep(int ms) 
{ 
	Sleep(ms); 
}

double __strtod(const char *_Str, char **_EndPtr)
{ 
	return strtod(_Str, _EndPtr);
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag;
 
	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);
 
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;
 
		tmpres -= 11644473600000000Ui64; 
		tmpres /= 10;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}
 
	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}
 
	return 0;
}
#endif

};