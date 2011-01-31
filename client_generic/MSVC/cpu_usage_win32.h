#pragma once
#include	"base.h"
#include	"MathBase.h"

#include "windows.h"
#include "../msvc/msvc_fix.h"
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "Timer.h"

class ESCpuUsage
{
private:
	FILETIME		m_LastIdleTime;
	FILETIME		m_LastKernelTime;
	FILETIME		m_LastUserTime;
	FILETIME		m_LastESKernelTime;
	FILETIME		m_LastESUserTime;
	Base::CTimer	m_Timer;
	fp8				m_LastCPUCheckTime;
public:
	ESCpuUsage(): m_LastCPUCheckTime(0)
	{
		m_Timer.Reset();
		m_LastCPUCheckTime = m_Timer.Time();
		GetSystemTimes( &m_LastIdleTime, &m_LastKernelTime, &m_LastUserTime );
	}
	bool GetCpuUsage(int &_total, int &_es)
	{
		FILETIME dummy1;
		FILETIME dummy2;
		FILETIME idleTime;
		FILETIME kernelTime;
		FILETIME userTime;
		LARGE_INTEGER usr;
		LARGE_INTEGER ker;
		LARGE_INTEGER idl;

		SYSTEM_INFO sysinfo;
		GetSystemInfo( &sysinfo );

		fp8 newtime = m_Timer.Time();
		if (GetProcessTimes(GetCurrentProcess(), &dummy1, &dummy2, &kernelTime, &userTime ) != 0)
		{
			usr.LowPart = userTime.dwLowDateTime - m_LastESUserTime.dwLowDateTime;
			usr.HighPart = userTime.dwHighDateTime- m_LastESUserTime.dwHighDateTime;
			ker.LowPart = kernelTime.dwLowDateTime - m_LastESKernelTime.dwLowDateTime;
			ker.HighPart = kernelTime.dwHighDateTime - m_LastESKernelTime.dwHighDateTime;

			fp8 period =  newtime - m_LastCPUCheckTime;
			if (period > 0.)
			{
				_es = int ( (ker.QuadPart + usr.QuadPart) * 100. / (period*1e+7) / sysinfo.dwNumberOfProcessors );
			}
			else
				_es = 0;
			_es = ::Base::Math::Clamped(_es, 0, 100);
			m_LastESKernelTime = kernelTime;
			m_LastESUserTime = userTime;
		} else
			return false;
		m_LastCPUCheckTime = newtime;

		if (GetSystemTimes( &idleTime, &kernelTime, &userTime ) != 0)
		{
			usr.LowPart = userTime.dwLowDateTime - m_LastUserTime.dwLowDateTime;
			usr.HighPart = userTime.dwHighDateTime- m_LastUserTime.dwHighDateTime;
			ker.LowPart = kernelTime.dwLowDateTime - m_LastKernelTime.dwLowDateTime;
			ker.HighPart = kernelTime.dwHighDateTime - m_LastKernelTime.dwHighDateTime;
			idl.LowPart = idleTime.dwLowDateTime - m_LastIdleTime.dwLowDateTime;
			idl.HighPart = idleTime.dwHighDateTime - m_LastIdleTime.dwHighDateTime;

			if (ker.QuadPart + usr.QuadPart != 0)
			{
				_total = int ( (ker.QuadPart + usr.QuadPart - idl.QuadPart) * 100 / (ker.QuadPart + usr.QuadPart) );
			}
			else
				_total = 0;
			_total = ::Base::Math::Clamped(_total, 0, 100);
			m_LastIdleTime = idleTime;
			m_LastKernelTime = kernelTime;
			m_LastUserTime = userTime;
		}
		else
			return false;
		return true;
	}
};