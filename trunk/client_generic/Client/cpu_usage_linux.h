#ifndef _CPU_USAGE_LINUX_H_
#define _CPU_USAGE_LINUX_H_

#include <sys/time.h>
#include <sys/resource.h>

#include <glibtop.h>
#include <glibtop/cpu.h>

class ESCpuUsage
{
	Base::CTimer	m_Timer;
	fp8				m_LastCPUCheckTime;
	
	fp8  m_LastSystemTime;
	fp8  m_LastESTime;
	
	glibtop *glibtopdata;
	glibtop_cpu m_lastProcessorInfo, m_currentProcessorInfo;
	
	int numProcessors;

public:
	ESCpuUsage(): m_LastCPUCheckTime(0)
	{

	  glibtopdata = glibtop_init();

	  m_Timer.Reset();
	  m_LastCPUCheckTime = m_Timer.Time();
	  
	  struct rusage r_usage;
	  if (!getrusage(RUSAGE_SELF, &r_usage)) {
		
	    m_LastESTime = (fp8)r_usage.ru_utime.tv_sec + (fp8)r_usage.ru_utime.tv_usec * 1e-6;
	    
	    m_LastESTime += (fp8)r_usage.ru_stime.tv_sec + (fp8)r_usage.ru_stime.tv_usec * 1e-6;
	    
	  }
	  
	  numProcessors = glibtopdata->ncpu + 1;
	  glibtop_get_cpu (&m_lastProcessorInfo);
	}
	
	virtual ~ESCpuUsage()
	{
	}

	bool GetCpuUsage(int &_total, int &_es)
	{
	  struct rusage r_usage;

		fp8 newtime = m_Timer.Time();

		fp8 period =  newtime - m_LastCPUCheckTime;
		if (period > 0.)
		{				
		  if (!getrusage(RUSAGE_SELF, &r_usage)) 
			{
				
			  fp8 utime = (fp8)r_usage.ru_utime.tv_sec + (fp8)r_usage.ru_utime.tv_usec * 1e-6;
			  
			  utime += (fp8)r_usage.ru_stime.tv_sec + (fp8)r_usage.ru_stime.tv_usec * 1e-6;
			  
			  _es = int ( ( utime - m_LastESTime ) * 100. / period );
			  
			  m_LastESTime = utime;
			}
			
		  float accInUse = 0.0f, accTotal = 0.0f;

		  glibtop_get_cpu (&m_currentProcessorInfo);
				
		  for(unsigned i = 0U; i < numProcessors; ++i)
		    {
		      float inUse, total;
		      
		      inUse = (
			       (m_currentProcessorInfo.xcpu_user[i]   - m_lastProcessorInfo.xcpu_user[i])
			       + (m_currentProcessorInfo.xcpu_sys[i] - m_lastProcessorInfo.xcpu_sys[i])
			       + (m_currentProcessorInfo.xcpu_nice[i] - m_lastProcessorInfo.xcpu_nice[i])
			       );
		      
		      total = inUse + (m_currentProcessorInfo.xcpu_idle[i] - m_lastProcessorInfo.xcpu_idle[i]);
		      
		      accInUse += inUse;
		      accTotal += total;
		    }
				
		  _total = (fp8)accInUse * 100. / (fp8)accTotal;
				
		  _es /= numProcessors;

		  memcpy(&m_lastProcessorInfo, &m_currentProcessorInfo, sizeof( glibtop_cpu ) );

		}
		else
		  {
		    _es = 0;
		    _total = 0;
		  }
		
		_es = ::Base::Math::Clamped(_es, 0, 100);
		_total = ::Base::Math::Clamped(_total, 0, 100);
		
		m_LastCPUCheckTime = newtime;

		return true;
	}
	
};


#endif
