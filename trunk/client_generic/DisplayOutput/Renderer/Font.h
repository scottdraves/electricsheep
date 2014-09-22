/*
	FONT.H
	Author: Stef.

	Font.
*/
#ifndef	__DAFONT_H_
#define	__DAFONT_H_

#include	<string>
#include	"base.h"
#include	"SmartPtr.h"

namespace	DisplayOutput
{

/*
	CFontDescription().

*/
class CFontDescription
{
	public:

		enum eStyle
		{
			Thin,
			Light,
			Normal,
			Bold,
			UberBold,
		};

	private:

		uint32		m_Height;
		eStyle		m_Style;
		bool		m_bItalic;
		bool		m_bUnderline;
		bool		m_bAntiAliased;
		std::string	m_TypeFace;

	public:
			//
			CFontDescription();
			~CFontDescription();

			//
			bool operator == (const CFontDescription &_rhs ) const;

			//
			void			Height( uint32 _h )			{	m_Height = _h;	};
			uint32	Height( void ) const		{	return( m_Height );	};

			//
			void			Style( const eStyle _w )	{	m_Style = _w;		};
			eStyle	Style( void ) const			{	return( m_Style );	};

			//
			void			Italic( const bool _b )		{	m_bItalic = _b;			};
			bool		Italic( void ) const		{	return( m_bItalic );	};

			//
			void			Underline( const bool _b )	{	m_bUnderline = _b;		};
			bool		Underline( void ) const		{	return( m_bUnderline );	};

			//
			void			AntiAliased( const bool _b )	{	m_bAntiAliased = _b;		};
			bool			AntiAliased( void ) const		{	return( m_bAntiAliased );	};

			//
			void				TypeFace( const std::string &_n )	{	m_TypeFace = _n;		};
			const std::string	&TypeFace() const					{	return( m_TypeFace );	};
};

MakeSmartPointers( CFontDescription );

/*
	CBaseFont.

*/
class	CBaseFont
{
	protected:

		CFontDescription	m_FontDescription;

	public:
			CBaseFont();
			virtual ~CBaseFont();

			enum eRenderStyle
			{
				Bottom = 1,
				Top,
				Center,
				Left,
				Right,
				VCenter,
				NoClip,
				ExpandTabs,
				WordBreak,
			};

			virtual	bool	Create() = PureVirtual;
	
			virtual void	Reupload() { };

			//
			void	FontDescription( const CFontDescription &_desc )	{	m_FontDescription = _desc;		};
			const CFontDescription FontDescription( void ) const		{	return( m_FontDescription );	};

};

MakeSmartPointers( CBaseFont );

}

#endif
