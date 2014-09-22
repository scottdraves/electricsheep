#ifndef _CPU_USAGE_MAC_H_
#define _CPU_USAGE_MAC_H_

#include <sys/resource.h>

#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>


class ESCpuUsage
{
	Base::CTimer	m_Timer;
	fp8				m_LastCPUCheckTime;
	
	fp8  m_LastESTime;
	
	processor_info_array_t m_lastProcessorInfo;
	mach_msg_type_number_t m_lastNumProcessorInfo;

	
public:
	ESCpuUsage(): m_LastCPUCheckTime(0)
	{
		m_Timer.Reset();
		m_LastCPUCheckTime = m_Timer.Time();
		
		struct rusage r_usage;
		if (!getrusage(RUSAGE_SELF, &r_usage)) {
		
				m_LastESTime = (fp8)r_usage.ru_utime.tv_sec + (fp8)r_usage.ru_utime.tv_usec * 1e-6;
				
				m_LastESTime += (fp8)r_usage.ru_stime.tv_sec + (fp8)r_usage.ru_stime.tv_usec * 1e-6;
			
		}
				
		processor_info_array_t processorInfo;
		mach_msg_type_number_t numProcessorInfo;
		natural_t numProcessors = 0U;

		kern_return_t err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &numProcessors, &processorInfo, &numProcessorInfo);
		if(err == KERN_SUCCESS) 
		{
			m_lastProcessorInfo = processorInfo;
			m_lastNumProcessorInfo = numProcessorInfo;
		}
		else 
		{
			m_lastProcessorInfo = NULL;
			m_lastNumProcessorInfo = 0;
		}

	}
	
	virtual ~ESCpuUsage()
	{
		if(m_lastProcessorInfo)
		{
			size_t lastProcessorInfoSize = sizeof(integer_t) * m_lastNumProcessorInfo;
			vm_deallocate(mach_task_self(), (vm_address_t)m_lastProcessorInfo, lastProcessorInfoSize);
		}
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
			
			processor_info_array_t processorInfo;
			mach_msg_type_number_t numProcessorInfo;
			natural_t numProcessors = 0U;

			kern_return_t err = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &numProcessors, &processorInfo, &numProcessorInfo);
			if(err == KERN_SUCCESS) 
			{
				float accInUse = 0.0f, accTotal = 0.0f;
				
				for(unsigned i = 0U; i < numProcessors; ++i)
				{
					float inUse, total;

					if(m_lastProcessorInfo) 
					{
						inUse = (
						  (processorInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER]   - m_lastProcessorInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER])
						+ (processorInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] - m_lastProcessorInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM])
						+ (processorInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE]   - m_lastProcessorInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE])
						);
						
						total = inUse + (processorInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE] - m_lastProcessorInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE]);
					} else {
						inUse = processorInfo[(CPU_STATE_MAX * i) + CPU_STATE_USER] + processorInfo[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] + processorInfo[(CPU_STATE_MAX * i) + CPU_STATE_NICE];
						total = inUse + processorInfo[(CPU_STATE_MAX * i) + CPU_STATE_IDLE];
					}
					
					accInUse += inUse;
					accTotal += total;
				}
				
				_total = static_cast<int>((fp8)accInUse * 100. / (fp8)accTotal);
				
				_es /= numProcessors;

				if(m_lastProcessorInfo)
				{
					size_t lastProcessorInfoSize = sizeof(integer_t) * m_lastNumProcessorInfo;
					vm_deallocate(mach_task_self(), (vm_address_t)m_lastProcessorInfo, lastProcessorInfoSize);
				}

				m_lastProcessorInfo = processorInfo;
				m_lastNumProcessorInfo = numProcessorInfo;
			}
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
