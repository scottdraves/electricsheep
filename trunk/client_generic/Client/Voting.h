#ifndef	_VOTING_H_
#define	_VOTING_H_

#include	"base.h"
#include	"Timer.h"
#include	"BlockingQueue.h"

/*
	CVote.
	Small class to handle voting of sheep.
*/
class	CVote
{	
	typedef struct
	{
		uint32	vid;
		uint8	vtype;
	} VotingInfo;
	
	boost::thread	*m_pThread;
	Base::CTimer	m_Timer;
	
	Base::CBlockingQueue<VotingInfo> m_Votings;

	void	ThreadFunc();

	fp8		m_Clock;

	public:
			CVote();
			virtual ~CVote();

			bool	Vote( const uint32 _id, const uint8 _type, const fp4 _duration );
};




#endif


