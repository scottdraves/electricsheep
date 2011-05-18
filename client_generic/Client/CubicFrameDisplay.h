#ifndef	_CUBICFRAMEDISPLAY_H_
#define	_CUBICFRAMEDISPLAY_H_

#include	"Rect.h"
#include	"Vector4.h"

/*
	CCubicFrameDisplay().
	Does a piecewise cubic interpolation between two frames using Mitchell Netravali reconstruction filter.
*/
class	CCubicFrameDisplay : public CFrameDisplay
{
	fp4 m_LastAlpha;
	DisplayOutput::spCShader m_spShader;

	//	The four frames.
	DisplayOutput::spCTextureFlat		m_spFrames[4];

	//	Simple ringbuffer...
	uint32	m_Frames[ 4 ];
	uint32	m_NumFrames;
	
	bool m_bWaitNextFrame;

	//	Mitchell Netravali Reconstruction Filter.
	fp4	MitchellNetravali( const fp4 _x, const fp4 _B, const fp4 _C )
	{
		float ax = fabs(_x);

		if( ax < 1 )
			return( (12 - 9 * _B - 6 * _C) * ax * ax * ax + (-18 + 12 * _B + 6 * _C) * ax * ax + (6 - 2 * _B)) / 6;
		else if( (ax >= 1) && (ax < 2) )
			return ((-_B - 6 * _C) * ax * ax * ax + (6 * _B + 30 * _C) * ax * ax + (-12 * _B - 48 * _C) * ax + (8 * _B + 24 * _C)) / 6;
		else
			return 0;
	}

	public:
			CCubicFrameDisplay( DisplayOutput::spCRenderer _spRenderer ) : CFrameDisplay( _spRenderer )
			{
				//	glsl fragment shader
				static const char *cubic_fragmentshaderGL2D = "\
					uniform sampler2D texUnit0;	\
					uniform sampler2D texUnit1;	\
					uniform sampler2D texUnit2;	\
					uniform sampler2D texUnit3;	\
					uniform vec4	weights;\
					uniform float	newalpha;\
					void main(void)\
					{\
						vec4 c0 = texture2D( texUnit0, gl_TexCoord[0].st );\
						vec4 c1 = texture2D( texUnit1, gl_TexCoord[0].st );\
						vec4 c2 = texture2D( texUnit2, gl_TexCoord[0].st );\
						vec4 c3 = texture2D( texUnit3, gl_TexCoord[0].st );\
						gl_FragColor = ( c0 * weights.x ) + ( c1 * weights.y ) + ( c2 * weights.z ) + ( c3 * weights.w ); \
						gl_FragColor.a = newalpha;\
					}";

				//	glsl fragment shader 2DRect
				static const char *cubic_fragmentshaderGL2DRect = "\
					uniform sampler2DRect texUnit0;	\
					uniform sampler2DRect texUnit1;	\
					uniform sampler2DRect texUnit2;	\
					uniform sampler2DRect texUnit3;	\
					uniform vec4	weights;\
					uniform float	newalpha;\
					void main(void)\
					{\
						vec4 c0 = texture2DRect( texUnit0, gl_TexCoord[0].st );\
						vec4 c1 = texture2DRect( texUnit1, gl_TexCoord[0].st );\
						vec4 c2 = texture2DRect( texUnit2, gl_TexCoord[0].st );\
						vec4 c3 = texture2DRect( texUnit3, gl_TexCoord[0].st );\
						gl_FragColor = ( c0 * weights.x ) + ( c1 * weights.y ) + ( c2 * weights.z ) + ( c3 * weights.w ); \
						gl_FragColor.a = newalpha;\
					}";

				//	hlsl vertexshader...
				static const char *cubic_vertexshader = "\
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

				//	hlsl fragmentshader.
				static const char *cubic_fragmentshaderDX = "\
					sampler2D texUnit0: register(s0);\
					sampler2D texUnit1: register(s1);\
					sampler2D texUnit2: register(s2);\
					sampler2D texUnit3: register(s3);\
					float4	weights;\
					float newalpha;\
					float4 main( float2 _uv : TEXCOORD0 ) : COLOR0\
					{\
						float4 c0 = tex2D( texUnit0, _uv );\
						float4 c1 = tex2D( texUnit1, _uv );\
						float4 c2 = tex2D( texUnit2, _uv );\
						float4 c3 = tex2D( texUnit3, _uv );\
						float4 c4 = ( c0 * weights.x ) + ( c1 * weights.y ) + ( c2 * weights.z ) + ( c3 * weights.w );\
						c4.a = newalpha;\
						return c4;\
					}";


				//	Compile the shader.
				if( _spRenderer->Type() == DisplayOutput::eDX9 )
					m_spShader = _spRenderer->NewShader( cubic_vertexshader, cubic_fragmentshaderDX );
				else
					m_spShader = _spRenderer->NewShader( NULL, ( _spRenderer->GetTextureTargetType() == DisplayOutput::eTexture2DRect ) ? cubic_fragmentshaderGL2DRect : cubic_fragmentshaderGL2D );

				if( !m_spShader )
					m_bValid = false;

				m_NumFrames = 0;

				m_Frames[0] = 0;
				m_Frames[1] = 1;
				m_Frames[2] = 2;
				m_Frames[3] = 3;
				
				m_bWaitNextFrame = false;
			}

			virtual ~CCubicFrameDisplay()
			{
			}

			//	Decode a frame every 1/_fpsCap seconds, store the previous 4 frames, and lerp between them.
			virtual bool	Update( ContentDecoder::spCContentDecoder _spDecoder, const fp8 _decodeFps, const fp8 _displayFps, ContentDecoder::sMetaData &_metadata )
			{
				fp4 currentalpha = m_LastAlpha;
				if (m_bWaitNextFrame)
				{
#if !defined(WIN32) && !defined(_MSC_VER)
					if ( !GrabFrame( _spDecoder, m_spFrames[ m_Frames[3] ], _metadata) )
					{
						return false;
					} else
					{
						m_MetaData = _metadata;
						m_LastAlpha = m_MetaData.m_Fade;
						currentalpha = m_LastAlpha;
					}
					
					Reset();
												
					m_bWaitNextFrame = false;
					m_NumFrames++;
#else
					if ( GrabFrame( _spDecoder, m_spFrames[ m_Frames[3] ], _metadata ) )
					{
						m_MetaData = _metadata;
						m_LastAlpha = m_MetaData.m_Fade;
						currentalpha = m_LastAlpha;
						Reset();
						m_NumFrames++;													
						m_bWaitNextFrame = false;
					}
#endif						
				}
				else
				{
					if( UpdateInterframeDelta( _decodeFps) )
					{
						//	Shift array back one step.
						uint32 tmp = m_Frames[ 0 ];
						m_Frames[ 0 ] = m_Frames[ 1 ];
						m_Frames[ 1 ] = m_Frames[ 2 ];
						m_Frames[ 2 ] = m_Frames[ 3 ];
						m_Frames[ 3 ] = tmp;

						//	... and fill the frontmost slot.
						if( !GrabFrame( _spDecoder, m_spFrames[ m_Frames[3] ], _metadata ) )
						{
							m_bWaitNextFrame = true;
#if !defined(WIN32) && !defined(_MSC_VER)
							return false;
#else
							m_NumFrames--;
#endif
						}
						else
						{
							m_MetaData = _metadata;
							m_LastAlpha = m_MetaData.m_Fade;
							currentalpha = m_LastAlpha;
						}

						m_NumFrames++;
					}
					else
					{
						currentalpha = (fp4)Base::Math::Clamped(m_LastAlpha + 
							Base::Math::Clamped(m_InterframeDelta/m_FadeCount, 0., 1./m_FadeCount)
							, 0., 1.);
					}
				}

				if (m_NumFrames > 0 && !m_spFrames[ m_Frames[3] ].IsNull())
				{
					//	Not enough frames decoded yet, display the latest one only.
					if( m_NumFrames < 4 )
					{
						m_spRenderer->SetTexture( m_spFrames[ m_Frames[3] ], 0 );
					}
					else
					{
						//	Enable the shader.
						m_spRenderer->SetShader( m_spShader );

						//	Set the 4 textures.
						for( uint32 i=0; i<4; i++ )
							m_spRenderer->SetTexture( m_spFrames[ m_Frames[i] ], i );

						//	B = 1,   C = 0   - cubic B-spline
						//	B = 1/3, C = 1/3 - nice
						//	B = 0,   C = 1/2 - Catmull-Rom spline.
						const fp4 B = 1.0f;
						const fp4 C = 0.0f;

						//	Set the filter weights...
						m_spShader->Set( "weights", MitchellNetravali( fp4(m_InterframeDelta) + 1.f, B, C ), MitchellNetravali( fp4(m_InterframeDelta), B, C ),
													MitchellNetravali( 1.f - fp4(m_InterframeDelta), B, C ), MitchellNetravali( 2.f - fp4(m_InterframeDelta), B, C ) );
						m_spShader->Set( "newalpha", currentalpha);
					}
					m_spRenderer->SetBlend( "alphablend" );
					m_spRenderer->Apply();
					m_spRenderer->DrawQuad( m_Size, Base::Math::CVector4( 0, 0, 0, currentalpha), m_spFrames[ m_Frames[3] ]->GetRect() );
				}

				return true;
			}

			virtual fp8 GetFps( fp8 _decodeFps, fp8 _displayFps )
			{
				return _displayFps;
			}
};

MakeSmartPointers( CCubicFrameDisplay );

#endif

