#pragma once
#include <direct.h>
#include <string>
#include <time.h>
#include <Windows.h>

extern "C" 
{ 

#if !defined(_MSC_VER) || (_MSC_VER < 1900)
	int snprintf(char *buffer, size_t count, const char *fmt, ...);
#endif
#ifdef _WIN64
	void ___chkstk();
	void usleep(int ms);
	double __strtod(const char *_Str, char **_EndPtr);

	struct timezone 
	{
		int  tz_minuteswest; /* minutes W of Greenwich */
		int  tz_dsttime;     /* type of dst correction */
	};

	int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

};

//#ifdef _DEBUG
//#include <crtdbg.h>
//#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
//#else
//#define DEBUG_NEW new
//#endif