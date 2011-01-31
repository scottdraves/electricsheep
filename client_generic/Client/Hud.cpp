#include	<string>
#include	<string.h>
#include	"Hud.h"
#include	"Settings.h"

namespace	Hud
{

/*
*/
CHudManager::CHudManager()
{
	m_Timer.Reset();
}


/*
*/
CHudManager::~CHudManager()
{
	m_EntryMap.clear();
}

/*
*/
bool	CHudManager::Add( const std::string _name, spCHudEntry _entry, fp8 _duration )
{
	_entry->SetTime( m_Timer.Time(), _duration );
	m_EntryMap[ _name ] = _entry;
	return true;
}

/*
*/
bool	CHudManager::Remove( const std::string _name )
{
	m_EntryMap.erase( _name );
	return true;
}

/*
*/
bool	CHudManager::Render( DisplayOutput::spCRenderer _spRenderer )
{
	if( _spRenderer == NULL )
		return false;

	_spRenderer->Reset( DisplayOutput::eEverything );
	_spRenderer->Orthographic();
	_spRenderer->Apply();

    std::map<std::string, spCHudEntry>::iterator i;
	for( i=m_EntryMap.begin(); i != m_EntryMap.end(); )
	{
		spCHudEntry e = i->second;
		bool	bRemove = false;

		if( e->Visible() )
		{
			if( !e->Render( m_Timer.Time(), _spRenderer ) )
				bRemove = true;
		}

		if( bRemove )
		{
			std::map<std::string, spCHudEntry>::iterator next = i;
			++next;
			m_EntryMap.erase(i);
			i = next;
		}
		else
			++i;
	}

	return true;
}

//
void	CHudManager::HideAll()
{
    std::map<std::string, spCHudEntry>::const_iterator i;
	for( i=m_EntryMap.begin(); i != m_EntryMap.end(); ++i )
	{
		spCHudEntry e = i->second;
		e->Visible( false );
	}
}

//
void	CHudManager::Toggle( const std::string _entry )
{
	bool bState = m_EntryMap[ _entry ]->Visible();
	HideAll();
	m_EntryMap[ _entry ]->Visible( !bState);
}

void	CHudManager::Hide( const std::string _entry )
{
	m_EntryMap[ _entry ]->Visible( false );
}

}
