#include <string>
#include "DisplayOutput.h"
#include "Exception.h"


namespace	DisplayOutput
{

//	Eventqueue to store all key, mouse & power events.
std::queue<spCEvent>	CDisplayOutput::m_EventQueue;

/*
*/
CDisplayOutput::CDisplayOutput() :  m_XPosition(10) , m_YPosition(10), m_Width(0), m_Height(0),
                                    m_bFullScreen(false), m_bVSync(false), m_bClosed(false)
{
}

/*
*/
CDisplayOutput::~CDisplayOutput()
{
	ClearEvents();
}

/*
*/
bool	CDisplayOutput::GetEvent( spCEvent &_spEvent )
{
    if( !m_EventQueue.empty() )
    {
        _spEvent = m_EventQueue.front();
        m_EventQueue.pop();
        return true;
    }

    return false;
}

/*
*/
void	CDisplayOutput::ClearEvents()
{
	while( !m_EventQueue.empty() )
		m_EventQueue.pop();
}
	
/*
*/
void	CDisplayOutput::AppendEvent( spCEvent _event )
{
	m_EventQueue.push( _event );
}



};
