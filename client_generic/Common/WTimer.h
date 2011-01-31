#ifndef	_WTIMER_H_
#define	_WTIMER_H_

#ifdef _TIMER_H_
#error "WTimer.h included not from Timer.h"
#endif

#include	<Windows.h>

namespace	Base
{

/*
	CWTimer.
*/
class CWTimer : public ITimer
{
	fp8		m_Time;				//	Current time in seconds.
	int64	m_TimeCounter;      //	Raw 64bit timer counter for time.
	int64	m_DeltaCounter;		//	Raw 64bit timer counter for delta.
	int64	m_Frequency;		//	Raw 64bit timer frequency.

	public:
			CWTimer()
			{
				QueryPerformanceFrequency( (LARGE_INTEGER *)&m_Frequency );
				Reset();
			}

			void	Reset()
			{
				QueryPerformanceCounter( (LARGE_INTEGER *)&m_TimeCounter );
				m_DeltaCounter = m_TimeCounter;
				m_Time = 0;
			}

			fp8	Time()
			{
				int64 counter;
				QueryPerformanceCounter( (LARGE_INTEGER *)&counter );
				m_Time += (fp8)(counter - m_TimeCounter) / (fp8)m_Frequency;
				m_TimeCounter = counter;
				return m_Time;
			}

			fp8	Delta()
			{
				int64 counter;
				QueryPerformanceCounter( (LARGE_INTEGER *)&counter );
				m_DeltaCounter = counter;
				return (fp8)(counter - m_DeltaCounter) / (fp8)m_Frequency;
			}

			fp8	Resolution()
			{
				return 1.0 / (fp8)m_Frequency;
			}

			static void Wait( const fp8 _seconds )
			{
				Sleep( int32(_seconds*1000) );
			}
};

typedef CWTimer CTimer;

};

#endif
