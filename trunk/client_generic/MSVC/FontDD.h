#pragma once
#ifndef	_FONTDD_H_
#define	_FONTDD_H_

#include	"base.h"
#include	"SmartPtr.h"
#include	"Font.h"
#include	<tchar.h>

namespace	DisplayOutput
{

/*
	CFontDX.

*/
class	CFontDD :	public CBaseFont
{
	HFONT	m_pDDFont;

	public:
			CFontDD( ) 
			{
				m_pDDFont = NULL;
			}
			virtual ~CFontDD() 
			{
				if (m_pDDFont != NULL)
				{
					DeleteObject(m_pDDFont);
					m_pDDFont = NULL;
				}
			}

			bool	Create()
			{
				LOGFONTA logfont = {0};
				switch ( m_FontDescription.Style() )
				{
					case CFontDescription::Thin:		logfont.lfWeight |= FW_THIN;		break;
					case CFontDescription::Light:		logfont.lfWeight |= FW_LIGHT;		break;
					case CFontDescription::Normal:		logfont.lfWeight |= FW_NORMAL;		break;
					case CFontDescription::Bold:		logfont.lfWeight |= FW_BOLD;		break;
					case CFontDescription::UberBold:	logfont.lfWeight |= FW_EXTRABOLD;	break;
				}
				logfont.lfHeight = m_FontDescription.Height();
				//HDC hdc = GetDC(NULL);
				//if (hdc != NULL)
				//{
				//	logfont.lfHeight = -MulDiv(m_FontDescription.Height(), GetDeviceCaps(hdc ,LOGPIXELSY), 72);
				//	ReleaseDC(NULL, hdc);
				//	logfont.lfHeight = -MulDiv(m_FontDescription.Height(), 72, 72) - 1;
				//}
				//else
				//{
				//	logfont.lfHeight = -MulDiv(m_FontDescription.Height(), 72, 72) - 1;
				//}
				logfont.lfItalic = m_FontDescription.Italic();
				logfont.lfQuality = m_FontDescription.AntiAliased() ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY;
				strcpy_s(logfont.lfFaceName, 32, m_FontDescription.TypeFace().c_str());

				m_pDDFont = CreateFontIndirectA(&logfont);

				return true;
			}

			HFONT	GetDDFont( void )	{	return(	m_pDDFont );	};
};

MakeSmartPointers( CFontDD );

};

#endif
