#ifndef	_CROSSFADE_H_
#define _CROSSFADE_H_

#include	"Hud.h"
#include	"Rect.h"

namespace	Hud
{

/*
	CFade.

*/
class	CCrossFade : public CHudEntry
{
	DisplayOutput::spCTextureFlat m_spTexture;
	bool m_bSkipped;
	bool m_bSkipToNext;
	uint32 m_currID;

	public:
			CCrossFade( uint32 width, uint32 height, bool skipToNext ) : CHudEntry( Base::Math::CRect(fp4(width), fp4(height) ) ), m_bSkipped(false), m_bSkipToNext(skipToNext)
			{
				DisplayOutput::spCImage tmpImage = new DisplayOutput::CImage();
								
				tmpImage->Create( width, height, DisplayOutput::eImage_RGBA8, 0, false );
				
				for (uint32 x = 0; x < width; x++)
				{
					for (uint32 y = 0; y < height; y++)
					{
						tmpImage->PutPixel(static_cast<int32>(x), static_cast<int32>(y), 0, 0, 0, 255);
					}
				}
				
				m_spTexture = g_Player().Renderer()->NewTextureFlat();
				m_spTexture->Upload( tmpImage );
			}

			virtual ~CCrossFade()
			{
			}

			//	Override to make it always visible.
			virtual bool	Visible() const	{	return true;	};
			
			void Reset()
			{
				m_bSkipped = false;
				m_currID = g_Player().GetCurrentPlayingID();
			}

			virtual bool	Render( const fp8 _time, DisplayOutput::spCRenderer _spRenderer )
			{
				if( !CHudEntry::Render( _time, _spRenderer ) )
					return false;
					
				if( m_spTexture == NULL )
					return false;

				DisplayOutput::spCRenderer spRenderer = g_Player().Renderer();

				spRenderer->Reset( DisplayOutput::eTexture | DisplayOutput::eShader );
				spRenderer->SetTexture( m_spTexture, 0 );
				spRenderer->SetBlend( "alphablend" );
				spRenderer->Apply();
				
				fp4 alpha = fp4(m_Delta * 4.0f);
				
				if (alpha > 1.0f)
				{
					if (!m_bSkipped && m_bSkipToNext)
					{
						//skip to another only if we are still in the same sheep as at the beginning.
						if ( m_currID == g_Player().GetCurrentPlayingID() )
							g_Player().SkipToNext();
						
						m_bSkipped = true;
					}
					
					if (alpha > 2.0f)
						alpha = 1.0f - ( alpha - 1.0f );
					else
						alpha = 1.0f;
				}

				spRenderer->DrawQuad( Base::Math::CRect( 1, 1 ), Base::Math::CVector4( 1,1,1, alpha ), m_spTexture->GetRect() );

				return true;
			}
};

MakeSmartPointers( CCrossFade );

};

#endif

