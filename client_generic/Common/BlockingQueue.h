#ifndef __BLOCKING_QUEUE__
#define __BLOCKING_QUEUE__

#include "boost/thread/thread.hpp"
#include "boost/thread/condition.hpp"
#include <deque>

namespace Base
{

template <typename T> class CBlockingQueue
{
public:
    typedef boost::shared_lock<boost::shared_mutex> reader_lock;
    typedef boost::unique_lock<boost::shared_mutex> writer_lock;
	typedef boost::upgrade_lock<boost::shared_mutex> upg_reader_lock;
	typedef boost::upgrade_to_unique_lock< boost::shared_mutex > upgrade_to_writer;

    
	//typedef boost::mutex::scoped_lock   scoped_lock;

	CBlockingQueue() { m_maxQueueElements = 0xFFFFFFFF; }

	bool push( const T& el, bool pushBack = true,  bool checkMax = true )
	{
		writer_lock lock( m_mutex );

		if ( checkMax && m_queue.size() == m_maxQueueElements )
		{
			while ( m_queue.size() == m_maxQueueElements )
				m_fullCond.wait(lock);
		}

		if ( pushBack )
			m_queue.push_back( el );
		else
			m_queue.push_front( el );
			
		m_emptyCond.notify_all();
		return true;
	}

	bool peek( T& el, bool wait = false, bool popFront = true )
	{
		upg_reader_lock lock(m_mutex);
		
		if ( wait )
		{
			while (m_queue.empty())
			{
				upgrade_to_writer wlock(lock);
				m_emptyCond.wait(m_mutex);
			}
		}
		else
		{
			if ( m_queue.empty() )
				return false;
		}
		
		if ( popFront )
		{
			el = m_queue.front();
			//m_queue.pop_front();
		}
		else
		{
			el = m_queue.back();
			//m_queue.pop_back();
		}
		
		if ( m_queue.size() < m_maxQueueElements )
			m_fullCond.notify_all();
		
		if (m_queue.empty())
			m_nonEmptyCond.notify_all();
		
		return true;
	}
    
    void peekn( T& el, size_t n )
    {
        reader_lock lock(m_mutex);
        
        el = m_queue[n];
    }

	bool pop( T& el, bool wait = false, bool popFront = true )
	{
		upg_reader_lock lock(m_mutex);
		
		if ( wait )
		{
			while (m_queue.empty())
			{
				upgrade_to_writer wlock(lock);
				m_emptyCond.wait(m_mutex);
			}
		}
		else
		{
			if (m_queue.empty())
			{
				m_nonEmptyCond.notify_all();
				return false;
			}
		}
		
		upgrade_to_writer wlock(lock);

		if ( popFront )
		{
			el = m_queue.front();
			m_queue.pop_front();
		}
		else
		{
			el = m_queue.back();
			m_queue.pop_back();
		}
		
		if ( m_queue.size() < m_maxQueueElements )
			m_fullCond.notify_all();
		
		if (m_queue.empty())
			m_nonEmptyCond.notify_all();
		
		return true;
	}

	bool empty()
	{
		reader_lock lock( m_mutex );
		return m_queue.empty();
	}

	size_t size()
	{
		reader_lock lock( m_mutex );
		return m_queue.size();
	}

	void clear( size_t leave )
	{
		writer_lock lock( m_mutex );
		
		if ( leave == 0 )
		{
			m_queue.clear();
			m_nonEmptyCond.notify_all();
		}
		else
		{
			size_t sz = m_queue.size();
			
			while (	sz > leave )
			{
				m_queue.pop_back();
				
				sz--;	
			}
		}
		
		if ( leave < m_maxQueueElements )
			m_fullCond.notify_all();
	}
	
	void setMaxQueueElements( size_t max )
	{
		writer_lock lock( m_mutex );
		
		m_maxQueueElements = max;
	}
	
	bool waitForEmpty()
	{
		upg_reader_lock lock( m_mutex );
		
		if (m_queue.empty())
			return true;
			
		upgrade_to_writer wlock(lock); 

		m_nonEmptyCond.wait( m_mutex );
		
		return true;
	}

private:
	boost::shared_mutex m_mutex;
	boost::condition m_fullCond;
	boost::condition m_emptyCond;
	boost::condition m_nonEmptyCond;
	size_t m_maxQueueElements;
	std::deque<T> m_queue;
};

};

#endif //__BLOCKING_QUEUE__
