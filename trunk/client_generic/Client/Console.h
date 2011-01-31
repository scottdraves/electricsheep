#ifndef	_CONSOLE_H_
#define _CONSOLE_H_

#include	"Hud.h"
#include	"Rect.h"
#include	"Font.h"

namespace	Hud
{

/*
	CConsole.

*/
class	CConsole : public CHudEntry
{
	protected:

		DisplayOutput::spCBaseFont	m_spFont;

	public:
			CConsole( Base::Math::CRect _rect ) : CHudEntry( _rect )	{};
			virtual ~CConsole()	{};

			virtual bool	Render( const fp8 _time, DisplayOutput::spCRenderer _spRenderer ) = PureVirtual;
};

MakeSmartPointers( CConsole );

};

#endif
