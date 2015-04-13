#ifndef	_FRAMEDISPLAY_H_
#define	_FRAMEDISPLAY_H_

#include	"TextureFlat.h"
#include	"Player.h"
#include	"Rect.h"
#include	"Vector4.h"

//#ifndef FRAME_DIAG
//#define FRAME_DIAG
//#endif

/*
	CFrameDisplay().
	Basic display handling, simply blits texture.
*/
class	CFrameDisplay
{
	//	To keep track of in-between frame times.
	fp8		m_Clock;
	fp8		m_Acc;
	fp8		m_T;

	fp4		m_LastAlpha;
    
    fp8     m_LastTexMoveClock;
    
    fp4     m_CurTexMoveOff;
    
    fp4     m_CurTexMoveDir;
    
    const fp8 TEX_MOVE_SECS = 60.f * 30.f; //30 minutes

	protected:
		fp8		m_FadeCount;

		ContentDecoder::sMetaData m_MetaData;
		//	Temporary storage for decoded videoframe.
		ContentDecoder::spCVideoFrame	m_spFrameData;
		DisplayOutput::spCImage		m_spImageRef;
		DisplayOutput::spCImage		m_spSecondImageRef;
		
		DisplayOutput::spCRenderer	m_spRenderer;

		//	Dimensions of the display surface.
		Base::Math::CRect	m_dispSize;
    
        //  texture Rect
        Base::Math::CRect   m_texRect;

		//	To keep track of elapsed time.
		Base::CTimer	m_Timer;

		//	Frame texture.
		DisplayOutput::spCTextureFlat m_spVideoTexture;
		
		DisplayOutput::spCTextureFlat m_spSecondVideoTexture;
    
        bool m_bPreserveAR;

		//	Grab a frame from the decoder and use it as a texture.
		bool	GrabFrame( ContentDecoder::spCContentDecoder _spDecoder, DisplayOutput::spCTextureFlat &_spTexture, DisplayOutput::spCTextureFlat &_spSecondTexture, ContentDecoder::sMetaData &_metadata )
		{
			//_metadata.m_Fade = 1.0f;
			m_MetaData = _metadata;
			if (m_spFrameData != NULL)
			{
				m_spFrameData->GetMetaData(_metadata);
				m_MetaData = _metadata;
			}
			//	Clear old frame data.
			m_spFrameData = NULL;

			//	Spin until we have a decoded frame from decoder.	(spin really?)
			m_spFrameData = _spDecoder->Frame();
			if( m_spFrameData != NULL )
			{
				m_spFrameData->GetMetaData(_metadata);
				m_MetaData = _metadata;
				if( m_spImageRef->GetWidth() != m_spFrameData->Width() || m_spImageRef->GetHeight() != m_spFrameData->Height() )
				{
					//	Frame differs in size, recreate ref image.
					m_spImageRef->Create( m_spFrameData->Width(), m_spFrameData->Height(), DisplayOutput::eImage_RGBA8, false, true );
				}

				if (_spTexture.IsNull())
					_spTexture = m_spRenderer->NewTextureFlat();
				
				if( _spTexture == NULL )
					return false;
				
				//	Set image texturedata and upload to texture.
				m_spImageRef->SetStorageBuffer( m_spFrameData->StorageBuffer() );
				_spTexture->Upload( m_spImageRef );
				
#ifdef FRAME_DIAG
				g_Log->Info( "Grabbing frame %ld/%ld from %ld (first)...prog - %f, seam - %d", _metadata.m_FrameIdx, _metadata.m_MaxFrameIdx, _metadata.m_SheepID, _metadata.m_TransitionProgress, _metadata.m_IsSeam );
#endif				
				
				ContentDecoder::spCVideoFrame spSecondFrameData = _metadata.m_SecondFrame;
				
				if (!spSecondFrameData.IsNull())
				{
					if( m_spSecondImageRef->GetWidth() != spSecondFrameData->Width() || m_spSecondImageRef->GetHeight() != spSecondFrameData->Height() )
					{
						//	Frame differs in size, recreate ref image.
						m_spSecondImageRef->Create( spSecondFrameData->Width(), spSecondFrameData->Height(), DisplayOutput::eImage_RGBA8, false, true );
					}

					if (_spSecondTexture.IsNull())
						_spSecondTexture = m_spRenderer->NewTextureFlat();
					
					if( _spSecondTexture != NULL )
					{
						//	Set image texturedata and upload to texture.
						m_spSecondImageRef->SetStorageBuffer( spSecondFrameData->StorageBuffer() );
						_spSecondTexture->Upload( m_spSecondImageRef );
						
#ifdef FRAME_DIAG
						ContentDecoder::sMetaData tmpMetaData;
						
						spSecondFrameData->GetMetaData(tmpMetaData);
						
						g_Log->Info( "Grabbing frame %ld/%d from %ld (second)...prog - %f, seam - %d", tmpMetaData.m_FrameIdx, tmpMetaData.m_MaxFrameIdx, tmpMetaData.m_SheepID, tmpMetaData.m_TransitionProgress, tmpMetaData.m_IsSeam );
#endif
					}
				}
				else
					_spSecondTexture = NULL;

			}
			else
			{
				g_Log->Warning( "failed to get frame..." );
				return false;
			}
			
			return true;
		}

		//	Do some math to figure out the delta between frames...
		bool	UpdateInterframeDelta( const fp8 _fpsCap )
		{
			fp8	newTime = m_Timer.Time();
			fp8 deltaTime = newTime - m_Clock;
			m_Clock = newTime;
			m_Acc += deltaTime;
			
			const fp8 dt = 1.0 / (fp8)_fpsCap;
			bool bCrossedFrame = false;

			//	Accumulated time is longer than the requested framerate, we crossed over to the next frame
			if( m_Acc >= dt )
				bCrossedFrame = true;

			//	Figure out the delta between _fpsCap frames to lerp with.
			while( m_Acc >= dt )
			{
				m_T += dt;
				m_Acc -= dt;
			}

			//	This is our inter-frame delta, > 0 < 1 <
			m_InterframeDelta = m_Acc / dt;
			
			return bCrossedFrame;
		}
		
		void Reset( void )
		{
			g_Log->Warning( "resetting interframe..." );
			
			m_Clock = m_Timer.Time();
			m_Acc = 0;
		}
		
		fp8		m_InterframeDelta;
		bool	m_bValid;

	public:
			CFrameDisplay( DisplayOutput::spCRenderer _spRenderer )
			{
				m_spVideoTexture = NULL;
				m_spSecondVideoTexture = NULL;
				m_spFrameData = NULL;
				m_Clock = 0;
				m_Acc = 0;
				m_T = 0;
				m_spRenderer = _spRenderer;
				m_spImageRef = new DisplayOutput::CImage();
				m_spSecondImageRef = new DisplayOutput::CImage();
				m_bValid = true;
				m_FadeCount = (fp8)g_Settings()->Get("settings.player.fadecount", 30);
                
                m_bPreserveAR = g_Settings()->Get("settings.player.preserve_AR", false);
                m_texRect = Base::Math::CRect( 1, 1 );
                m_LastTexMoveClock = -1;
                m_CurTexMoveOff = 0;
                m_CurTexMoveDir = 1.;
			}

			virtual ~CFrameDisplay()
			{
				m_spVideoTexture = NULL;
				m_spSecondVideoTexture = NULL;
			}

			bool Valid()	{	return m_bValid;	};

			//
			void	SetDisplaySize( const uint32 _w, const uint32 _h )
			{
				m_dispSize = Base::Math::CRect( _w, _h );
                m_CurTexMoveOff = 0.f;
			}

			//	Decode a frame, and render it.
			virtual bool	Update( ContentDecoder::spCContentDecoder _spDecoder, const fp8 _decodeFps, const fp8 /*_displayFps*/, ContentDecoder::sMetaData &_metadata )
			{
				fp4 currentalpha = m_LastAlpha;
				bool isSeam = false;
				
                //making static analyzer happy...
                (void)currentalpha;
                
   				if( UpdateInterframeDelta( _decodeFps ) )
   				{
#if !defined(WIN32) && !defined(_MSC_VER)
					if (!GrabFrame( _spDecoder, m_spVideoTexture, m_spSecondVideoTexture, _metadata))
					{
						return false;
					}
					else
					{
						m_MetaData = _metadata;
						m_LastAlpha = m_MetaData.m_Fade;
						currentalpha = m_LastAlpha;
						isSeam = m_MetaData.m_IsSeam;
					}
#else
					if (GrabFrame( _spDecoder, m_spVideoTexture, m_spSecondVideoTexture, _metadata))
					{
						m_MetaData = _metadata;
						m_LastAlpha = m_MetaData.m_Fade;
						currentalpha = m_LastAlpha;
						isSeam = m_MetaData.m_IsSeam;
					}
#endif
   				}
				else
				{
					currentalpha = (fp4)Base::Math::Clamped(m_LastAlpha + 
						Base::Math::Clamped(m_InterframeDelta/m_FadeCount, 0., 1./m_FadeCount)
						, 0., 1.);
				}
				
				if (isSeam)
				{
					m_spSecondVideoTexture = NULL;
				}
				
				if ( m_spVideoTexture.IsNull() )
					return false;

				//	Bind texture and render a quad covering the screen.
				m_spRenderer->SetBlend( "alphablend" );
				m_spRenderer->SetTexture( m_spVideoTexture, 0 );
				m_spRenderer->Apply();

                //UpdateInterframeDelta( _decodeFps );
				
				fp4 transCoef = m_MetaData.m_TransitionProgress / 100.0f;

                UpdateTexRect( m_spVideoTexture->GetRect() );
                
                m_spRenderer->DrawQuad( m_texRect, Base::Math::CVector4( 1,1,1, currentalpha * (1.0f - transCoef) ), m_spVideoTexture->GetRect() );
				
				if (!m_spSecondVideoTexture.IsNull())
				{
					//	Bind the second texture and render a quad covering the screen.
					m_spRenderer->SetTexture( m_spSecondVideoTexture, 0 );
					m_spRenderer->Apply();
                    
                    m_spRenderer->DrawQuad( m_texRect, Base::Math::CVector4( 1,1,1, currentalpha * transCoef ), m_spVideoTexture->GetRect() );
				}

				return true;
			}
			
			virtual fp8 GetFps( fp8 /*_decodeFps*/, fp8 _displayFps )
			{
				return _displayFps;
			}
    
            virtual void UpdateTexRect(const Base::Math::CRect& texDim)
            {
                m_texRect.m_X0 = 0.f;
                m_texRect.m_Y0 = 0.f;
                m_texRect.m_X1 = 1.f;
                m_texRect.m_Y1 = 1.f;
                
                if (!m_bPreserveAR)
                    return;
                
                bool landscape = true;
                
                fp4 r1 = (fp4)m_dispSize.Width() / (fp4)m_dispSize.Height();
                fp4 r2 = texDim.Width() / texDim.Height();
                
                if (r2 > r1)
                {
                    r1 = 1.f / r1;
                    r2 = 1.f / r2;
                    landscape = false;
                }
                
                fp4 bars =  (r1 - r2) / (2 * r1);
                    
                if (bars > 0)
                {
                    fp8 acttm = m_Timer.Time();
                    
                    if (m_LastTexMoveClock < 0)
                    {
                        m_LastTexMoveClock = acttm;
                    }
                    
                    if ((acttm - m_LastTexMoveClock) > TEX_MOVE_SECS)
                    {
                        m_CurTexMoveOff += bars / 20.f * m_CurTexMoveDir;
                        m_LastTexMoveClock = acttm;
                    }
                    
                    fp4 *a = NULL, *b = NULL;
                    
                    if (landscape)
                    {
                        a = &m_texRect.m_X0;
                        b = &m_texRect.m_X1;
                    }
                    else
                    {
                        a = &m_texRect.m_Y0;
                        b = &m_texRect.m_Y1;
                    }
                    
                    if (a == NULL || b == NULL)
                        return;
                    
                    *a  += bars + m_CurTexMoveOff;
                    *b  -= bars - m_CurTexMoveOff;
                    
                    if (*a <= 0.f)
                    {
                        *b += -*a;
                        *a += -*a;
                        m_CurTexMoveOff -= bars / 20.f * m_CurTexMoveDir;
                        m_CurTexMoveDir = -m_CurTexMoveDir;
                    }
                    
                    if (*b >= 1.f)
                    {
                        *a -= *b - 1.f;
                        *b -= *b - 1.f;
                        m_CurTexMoveOff -= bars / 20.f * m_CurTexMoveDir;
                        m_CurTexMoveDir = -m_CurTexMoveDir;
                    }

                }
                
            }
};

MakeSmartPointers( CFrameDisplay );

#endif

