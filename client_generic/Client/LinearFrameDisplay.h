#ifndef	_LINEARFRAMEDISPLAY_H_
#define	_LINEARFRAMEDISPLAY_H_

#include	"Shader.h"
#include	"Rect.h"
#include	"Vector4.h"

/*
	CLinearFrameDisplay().
	Does a piecewise linear interpolation between two frames.
*/
class	CLinearFrameDisplay : public CFrameDisplay
{
	static const uint32 kMaxFrames = 2;
	
	fp4 m_LastAlpha;
	//	Pixelshader.
	DisplayOutput::spCShader m_spShader;

	//	The two frames.
	DisplayOutput::spCTextureFlat	m_spFrames[ 2 * kMaxFrames ];
	int32	m_State;

	bool m_bWaitNextFrame;

	public:
			CLinearFrameDisplay( DisplayOutput::spCRenderer _spRenderer ) : CFrameDisplay( _spRenderer )
			{
				m_State = 0;

				//	Pixelshader to lerp between two textures.
				static const char *linear_pixelshaderGL2D = "\
					uniform float delta;\
					uniform sampler2D texUnit1;	\
					uniform sampler2D texUnit2;	\
					uniform sampler2D texUnit3; \
					uniform sampler2D texUnit4; \
					uniform float newalpha;\
					uniform float transPct;\
					void main(void)\
					{\
						vec4 c1 = texture2D( texUnit1, gl_TexCoord[0].st );\
						vec4 c2 = texture2D( texUnit2, gl_TexCoord[0].st );\
						vec4 fc1 = mix( c1, c2, delta );\
						\
						c1 = texture2D( texUnit3, gl_TexCoord[0].st );\
						c2 = texture2D( texUnit4, gl_TexCoord[0].st );\
						vec4 fc2 = mix( c1, c2, delta );\
						\
						gl_FragColor = mix( fc1, fc2, transPct / 100.0 );\
						gl_FragColor.a = newalpha;\
					}";

				//	Pixelshader to lerp between two textures.
				static const char *linear_pixelshaderGL2DRect = "\
					uniform float delta;\
					uniform sampler2DRect texUnit1;	\
					uniform sampler2DRect texUnit2;	\
					uniform sampler2DRect texUnit3; \
					uniform sampler2DRect texUnit4; \
					uniform float newalpha;\
					uniform float transPct;\
					void main(void)\
					{\
						vec4 c1 = texture2DRect( texUnit1, gl_TexCoord[0].st );\
						vec4 c2 = texture2DRect( texUnit2, gl_TexCoord[0].st );\
						vec4 fc1 = mix( c1, c2, delta );\
						\
						c1 = texture2DRect( texUnit3, gl_TexCoord[0].st );\
						c2 = texture2DRect( texUnit4, gl_TexCoord[0].st );\
						vec4 fc2 = mix( c1, c2, delta );\
						\
						gl_FragColor = mix( fc1, fc2, transPct / 100.0 );\
						gl_FragColor.a = newalpha;\
					}";

				//	vertexshader...
				static const char *linear_vertexshader = "\
					float4x4 WorldViewProj: WORLDVIEWPROJECTION;\
					struct VS_OUTPUT\
					{\
						float4 Pos  : POSITION;\
						float2 Tex	: TEXCOORD0;\
					};\
					VS_OUTPUT main( float4 Pos : POSITION, float2 Tex : TEXCOORD0 )\
					{\
					  VS_OUTPUT Out = (VS_OUTPUT)0;\
					  Out.Pos = mul( Pos, WorldViewProj );\
					  Out.Tex = Tex;\
					  return Out;\
					}";

				//	Pixelshader to lerp between two textures.
				static const char *linear_pixelshaderDX = "\
					float delta;\
					float newalpha;\
					float transPct;\
					sampler2D texUnit1: register(s1);\
					sampler2D texUnit2: register(s2);\
					sampler2D texUnit3: register(s3);\
					sampler2D texUnit4: register(s4);\
					float4 main( float2 _uv : TEXCOORD0 ) : COLOR0\
					{\
						float4 c1 = tex2D( texUnit1, _uv );\
						float4 c2 = tex2D( texUnit2, _uv );\
						float4 c3 = lerp( c1, c2, delta );\
						c1 = tex2D( texUnit3, _uv );\
						c2 = tex2D( texUnit4, _uv );\
						float4 c4 = lerp( c1, c2, delta );\
						\
						float4 c5 = lerp( c3, c4, transPct / 100.0 );\
						c5.a = newalpha;\
						return c5;\
					}";

				//	Compile the shader.
				if( _spRenderer->Type() == DisplayOutput::eDX9 )
					m_spShader = _spRenderer->NewShader( linear_vertexshader, linear_pixelshaderDX );
				else
					m_spShader = _spRenderer->NewShader( NULL, ( _spRenderer->GetTextureTargetType() == DisplayOutput::eTexture2DRect ) ? linear_pixelshaderGL2DRect : linear_pixelshaderGL2D );

				if( !m_spShader )
					m_bValid = false;
			}

			virtual ~CLinearFrameDisplay()
			{
			}

			//	Decode a frame every 1/_fpsCap seconds, store the previous frame, and lerp between them.
			virtual bool	Update( ContentDecoder::spCContentDecoder _spDecoder, const fp8 _decodeFps, const fp8 _displayFps, ContentDecoder::sMetaData &_metadata )
			{
				fp4 currentalpha = m_LastAlpha;
				bool frameGrabbed = false;
				bool isSeam = false;
				
				if (m_bWaitNextFrame)
				{
#if !defined(WIN32) && !defined(_MSC_VER)
					if ( !GrabFrame( _spDecoder, m_spFrames[ m_State ], m_spFrames[ m_State + kMaxFrames ], _metadata ) )
					{
						return false;
					}
					else
					{
						frameGrabbed = true;
						
						Reset();
					
						m_bWaitNextFrame = false;

					}
					
#else
					if ( GrabFrame( _spDecoder, m_spFrames[ m_State ], m_spFrames[ m_State + kMaxFrames ], _metadata ) )
					{
						frameGrabbed = true;
						
						Reset();	
						
						m_bWaitNextFrame = false;
					}
#endif
				}
				else
				{
					if( UpdateInterframeDelta( _decodeFps ) )
					{
						m_State ^= 1;

						if( !GrabFrame( _spDecoder, m_spFrames[ m_State ], m_spFrames[ m_State + kMaxFrames ], _metadata ) )
						{
							m_bWaitNextFrame = true;
#if !defined(WIN32) && !defined(_MSC_VER)
							return false;
#endif
						}
						else
							frameGrabbed = true;
					}
					else
					{
						currentalpha = (fp4)Base::Math::Clamped(m_LastAlpha + 
							Base::Math::Clamped(m_InterframeDelta/m_FadeCount, 0., 1./m_FadeCount)
							, 0., 1.);
					}

				}
				
				if (frameGrabbed)
				{
					m_MetaData = _metadata;
					m_LastAlpha = m_MetaData.m_Fade;
					currentalpha = m_LastAlpha;
					
					isSeam = _metadata.m_IsSeam;
				}


				if ( m_spFrames[ m_State ] != NULL )
				{
					Base::Math::CRect texRect;
					
					if (isSeam)
					{
						m_spFrames[ !m_State ] = m_spFrames[ !m_State + kMaxFrames ];
						
						m_spFrames[ !m_State + kMaxFrames ] = NULL;							
					}
					
					m_spRenderer->SetShader( m_spShader );
					m_spRenderer->SetBlend( "alphablend" );

					//	Only one frame so far, let's display it normally.
					if( m_spFrames[ m_State ^ 1 ] == NULL )
					{
						//	Bind texture and render a quad covering the screen.
						m_spRenderer->SetTexture( m_spFrames[ m_State ], 1 );
						m_spRenderer->SetTexture( m_spFrames[ m_State ], 2 );
						
						if (!m_spFrames[ m_State + kMaxFrames ].IsNull())
						{
							m_spRenderer->SetTexture( m_spFrames[ m_State + kMaxFrames ], 2 );
							m_spRenderer->SetTexture( m_spFrames[ m_State + kMaxFrames ], 3 );
						}
					}
					else
					{
						m_spRenderer->SetTexture( m_spFrames[ 0 ], (m_State ^ 1) + 1);
						m_spRenderer->SetTexture( m_spFrames[ 1 ], m_State + 1);
						
						if (!m_spFrames[ m_State + kMaxFrames ].IsNull())
						{
							m_spRenderer->SetTexture( m_spFrames[ 2 ], (m_State ^ 1) + kMaxFrames + 1);
							m_spRenderer->SetTexture( m_spFrames[ 3 ], m_State + kMaxFrames + 1);
						}
					}
					texRect = m_spFrames[ m_State ]->GetRect();
					m_spShader->Set( "delta", (fp4)m_InterframeDelta );
					m_spShader->Set( "newalpha", (fp4)currentalpha );
					m_spShader->Set( "transPct", m_MetaData.m_TransitionProgress);
					m_spRenderer->Apply();
					
					
					m_spRenderer->DrawQuad( m_Size, Base::Math::CVector4( 1, 1, 1, currentalpha ), texRect );
				}

				return true;
			}

			virtual fp8 GetFps( fp8 _decodeFps, fp8 _displayFps )
			{
				return _displayFps;
			}
};

MakeSmartPointers( CLinearFrameDisplay );

#endif

