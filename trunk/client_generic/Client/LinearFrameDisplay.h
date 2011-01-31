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
	fp4 m_LastAlpha;
	//	Pixelshader.
	DisplayOutput::spCShader m_spShader;

	//	The two frames.
	DisplayOutput::spCTextureFlat	m_spFrames[2];
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
					uniform float newalpha;\
					void main(void)\
					{\
						vec4 c1 = texture2D( texUnit1, gl_TexCoord[0].st );\
						vec4 c2 = texture2D( texUnit2, gl_TexCoord[0].st );\
						gl_FragColor = mix( c1, c2, delta );\
						gl_FragColor.a = newalpha;\
					}";

				//	Pixelshader to lerp between two textures.
				static const char *linear_pixelshaderGL2DRect = "\
					uniform float delta;\
					uniform sampler2DRect texUnit1;	\
					uniform sampler2DRect texUnit2;	\
					uniform float newalpha;\
					void main(void)\
					{\
						vec4 c1 = texture2DRect( texUnit1, gl_TexCoord[0].st );\
						vec4 c2 = texture2DRect( texUnit2, gl_TexCoord[0].st );\
						gl_FragColor = mix( c1, c2, delta );\
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
					sampler2D texUnit1: register(s0);\
					sampler2D texUnit2: register(s1);\
					float4 main( float2 _uv : TEXCOORD0 ) : COLOR0\
					{\
						float4 c1 = tex2D( texUnit1, _uv );\
						float4 c2 = tex2D( texUnit2, _uv );\
						float4 c3 = lerp( c1, c2, delta );\
						c3.a = newalpha;\
						return c3;\
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
				if (m_bWaitNextFrame)
				{
#if !defined(WIN32) && !defined(_MSC_VER)
					if ( !GrabFrame( _spDecoder, m_spFrames[ m_State ], _metadata ) )
					{
						return false;
					}
					else
					{
						m_MetaData = _metadata;
						m_LastAlpha = m_MetaData.m_Fade;
					}
					
					Reset();
					
					m_bWaitNextFrame = false;
#else
					if ( GrabFrame( _spDecoder, m_spFrames[ m_State ], _metadata ) )
					{
						m_MetaData = _metadata;
						m_LastAlpha = m_MetaData.m_Fade;
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

						if( !GrabFrame( _spDecoder, m_spFrames[ m_State ], _metadata ) )
						{
							m_bWaitNextFrame = true;
#if !defined(WIN32) && !defined(_MSC_VER)
							return false;
#endif
						}
						else
						{
							m_MetaData = _metadata;
							m_LastAlpha = m_MetaData.m_Fade;
						}
					}
				}

				if ( m_spFrames[ m_State ] != NULL )
				{
					Base::Math::CRect texRect;
					
					//	Only one frame so far, let's display it normally.
					if( m_spFrames[ m_State ^ 1 ] == NULL )
					{
						//	Bind texture and render a quad covering the screen.
						m_spRenderer->SetBlend( "alphablend" );
						m_spRenderer->SetTexture( m_spFrames[ m_State ], 0 );
						m_spRenderer->Apply();
						texRect = m_spFrames[ m_State ]->GetRect();
					}
					else
					{
						m_spRenderer->SetShader( m_spShader );
						m_spRenderer->SetBlend( "alphablend" );
						m_spRenderer->SetTexture( m_spFrames[ 0 ], m_State ^ 1 );
						m_spRenderer->SetTexture( m_spFrames[ 1 ], m_State );
						m_spShader->Set( "delta", (fp4)m_InterframeDelta );
						m_spShader->Set( "newalpha", (fp4)m_LastAlpha );
						m_spRenderer->Apply();
						texRect = m_spFrames[ 0 ]->GetRect();
					}
					
					
					m_spRenderer->DrawQuad( m_Size, Base::Math::CVector4( 0, 0, 0, m_LastAlpha ), texRect );
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

