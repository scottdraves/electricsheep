#ifndef	_STARTUPSCREEN_H_
#define _STARTUPSCREEN_H_

#include	<sstream>
#include	"Hud.h"
#include	"Rect.h"
#include	"Console.h"

namespace	Hud
{

class	CStartupScreen : public CHudEntry
{
	std::string m_StartupMessage;
	DisplayOutput::CFontDescription m_Desc;
	DisplayOutput::spCBaseFont	m_spFont;
	DisplayOutput::spCImage		m_spImageRef;
	DisplayOutput::spCTextureFlat m_spVideoTexture;
	
	Base::Math::CRect m_LogoSize;
	fp4 m_MoveMessageCounter;

	public:
			CStartupScreen( Base::Math::CRect _rect, const std::string &_FontName, const uint32 _fontHeight ) : CHudEntry( _rect )
			{
				DisplayOutput::CFontDescription	fontDesc;

				m_Desc.AntiAliased( true );
				m_Desc.Height( _fontHeight );
				m_Desc.Style( DisplayOutput::CFontDescription::Normal );
				m_Desc.Italic( false );
				m_Desc.TypeFace( _FontName );

				m_spFont = g_Player().Renderer()->NewFont( m_Desc );
				m_StartupMessage = "No Sheep downloaded yet, this should take less than a minute\nbut might take several hours.  Please see ElectricSheep.org\nto learn more, or press F1 for help.";
				m_spImageRef = new DisplayOutput::CImage();
				m_spImageRef->Create(256, 256, DisplayOutput::eImage_RGBA8, false, true );
#ifndef LINUX_GNU
				m_spImageRef->Load(g_Settings()->Get( "settings.app.InstallDir", std::string(".\\") ) + "electricsheep-smile.png", false);
#else
				m_spImageRef->Load(g_Settings()->Get( "settings.app.InstallDir", std::string("") ) + "electricsheep-smile.png", false);
#endif
				fp4 aspect = g_Player().Display()->Aspect();
				m_LogoSize.m_X0 = 0.f;
				m_LogoSize.m_X1 = 0.2f * aspect;
				m_LogoSize.m_Y0 = 0.f;
				m_LogoSize.m_Y1 = 0.2f;

				m_spVideoTexture = NULL;
				m_MoveMessageCounter = 0.;
			}

			virtual ~CStartupScreen()
			{
				m_spVideoTexture = NULL;
			}

			bool	Render( const fp8 _time, DisplayOutput::spCRenderer _spRenderer )
			{
				CHudEntry::Render( _time, _spRenderer );

				if (m_bServerMessageStartTimer == false)
				{
					m_bServerMessageStartTimer = true;
					m_ServerMessageStartTimer = boost::posix_time::second_clock::local_time();
				}

				// draw picture

				DisplayOutput::spCImage tmpImage = new DisplayOutput::CImage();
				if( m_spImageRef.IsNull() == false )
				{
					m_spVideoTexture = _spRenderer->NewTextureFlat();
					m_spVideoTexture->Upload( m_spImageRef );
				}

				if ( m_spVideoTexture.IsNull() )
					return false;
				
				_spRenderer->Reset( DisplayOutput::eTexture | DisplayOutput::eShader);
				_spRenderer->SetBlend( "alphablend" );
				_spRenderer->SetTexture( m_spVideoTexture, 0 );
				_spRenderer->Apply();

				Base::Math::CRect rr;
				rr.m_X0 = 0.5f - (m_LogoSize.Width());
				rr.m_Y0 = 0.5f - (m_LogoSize.Height()) + m_MoveMessageCounter;
				rr.m_X1 = 0.5f + (m_LogoSize.Width());
				rr.m_Y1 = 0.5f + (m_LogoSize.Height()) + m_MoveMessageCounter;

				_spRenderer->DrawQuad( rr, Base::Math::CVector4( 1,1,1,1 ),  m_spVideoTexture->GetRect() );

				// draw text

				//fp4 step = (fp4)m_Desc.Height() / (fp4)_spRenderer->Display()->Height();
				fp4	pos = 0;
				fp4 edge = 24 / (fp4)_spRenderer->Display()->Width();

				Base::Math::CRect	extent;
				Base::Math::CVector2 size = g_Player().Renderer()->GetTextExtent( m_spFont, m_StartupMessage );
				extent = extent.Union( Base::Math::CRect( 0, 0, size.m_X+(edge*2), size.m_Y+(edge*2) ) );

				boost::posix_time::time_duration td = boost::posix_time::second_clock::local_time() - m_ServerMessageStartTimer;
				if (td.hours() >= 1)
				{
					m_MoveMessageCounter += 0.0005f;
					if (m_MoveMessageCounter >= 1.f)
						m_MoveMessageCounter -= 1.f + edge*2 + fp4(size.m_Y) + 0.2f;
				}

				//	Draw quad.
				_spRenderer->Reset( DisplayOutput::eTexture | DisplayOutput::eShader | DisplayOutput::eBlend );

				Base::Math::CRect r( 0.5f - (extent.Width()*0.5f), extent.m_Y0 + m_MoveMessageCounter + 0.2f,
									 0.5f + (extent.Width()*0.5f), extent.m_Y1 + m_MoveMessageCounter + 0.2f);

				_spRenderer->SetBlend( "alphablend" );
				_spRenderer->Apply();
				_spRenderer->DrawSoftQuad( r, Base::Math::CVector4( 0, 0, 0, 0.5f ), 16 );
				
				//dasvo - terrible hack - redo!!
				if (!m_spFont.IsNull())
					m_spFont->Reupload();
				
				pos = edge;
				_spRenderer->Text( m_spFont, m_StartupMessage, Base::Math::CVector4( 1, 1, 1, 1 ), Base::Math::CRect( r.m_X0+edge, r.m_Y0+edge, r.m_X1, r.m_Y1 ), 0 );

				return true;
			}
};

MakeSmartPointers( CStartupScreen );

};
#endif
