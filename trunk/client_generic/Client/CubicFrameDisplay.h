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
	static const uint32 kMaxFrames = 4;
	
	fp4 m_LastAlpha;
	DisplayOutput::spCShader m_spShader;

	//	The four frames.
	DisplayOutput::spCTextureFlat		m_spFrames[ 2 * kMaxFrames ];

	//	Simple ringbuffer...
	uint32	m_Frames[ kMaxFrames ];
	uint32	m_NumFrames;
	uint32  m_NumSecondFrames;
	
	bool m_bWaitNextFrame;

	//	Mitchell Netravali Reconstruction Filter.
	fp4	MitchellNetravali( const fp4 _x, const fp4 _B, const fp4 _C )
	{
		float ax = fabsf(_x);

		if( ax < 1.f )
			return( (12.f - 9.f * _B - 6.f * _C) * ax * ax * ax + (-18.f + 12.f * _B + 6.f * _C) * ax * ax + (6.f - 2.f * _B)) / 6.f;
		else if( (ax >= 1.f) && (ax < 2.f) )
			return ((-_B - 6.f * _C) * ax * ax * ax + (6.f * _B + 30.f * _C) * ax * ax + (-12.f * _B - 48.f * _C) * ax + (8.f * _B + 24.f * _C)) / 6.f;
		else
			return 0.f;
	}

	public:
			CCubicFrameDisplay( DisplayOutput::spCRenderer _spRenderer ) : CFrameDisplay( _spRenderer )
			{
				//	glsl fragment shader
				static const char *cubic_fragmentshaderGL2D = "\
					uniform sampler2D texUnit1;	\
					uniform sampler2D texUnit2;	\
					uniform sampler2D texUnit3;	\
					uniform sampler2D texUnit4;	\
					uniform sampler2D texUnit5;	\
					uniform sampler2D texUnit6;	\
					uniform sampler2D texUnit7;	\
					uniform sampler2D texUnit8;	\
					uniform vec4	weights;\
					uniform float	newalpha;\
					uniform float	transPct;\
					void main(void)\
					{\
						vec4 pos, c1, c2, c3, c4;\
						vec4 fc1, fc2;\
						\
						pos =  gl_TexCoord[0];\
						c1 = texture2D( texUnit1, pos.st );\
						c2 = texture2D( texUnit2, pos.st );\
						c3 = texture2D( texUnit3, pos.st );\
						c4 = texture2D( texUnit4, pos.st );\
						\
						fc1 = ( c1 * weights.x ) + ( c2 * weights.y ) + ( c3 * weights.z ) + ( c4 * weights.w );\
						\
						c1 = texture2D( texUnit5, pos.st );\
						c2 = texture2D( texUnit6, pos.st );\
						c3 = texture2D( texUnit7, pos.st );\
						c4 = texture2D( texUnit8, pos.st );\
						\
						fc2 = ( c1 * weights.x ) + ( c2 * weights.y ) + ( c3 * weights.z ) + ( c4 * weights.w );\
						\
						gl_FragColor = mix(fc1, fc2, transPct / 100.0); \
						gl_FragColor.a = newalpha;\
					}";

				//	glsl fragment shader 2DRect
				static const char *cubic_fragmentshaderGL2DRect = "\
					uniform sampler2DRect texUnit1;	\
					uniform sampler2DRect texUnit2;	\
					uniform sampler2DRect texUnit3;	\
					uniform sampler2DRect texUnit4;	\
					uniform sampler2DRect texUnit5;	\
					uniform sampler2DRect texUnit6;	\
					uniform sampler2DRect texUnit7;	\
					uniform sampler2DRect texUnit8;	\
					uniform vec4	weights;\
					uniform float	newalpha;\
					uniform float	transPct;\
					void main(void)\
					{\
						vec4 pos, c1, c2, c3, c4;\
						vec4 fc1, fc2;\
						\
						pos =  gl_TexCoord[0];\
						c1 = texture2DRect( texUnit1, pos.st );\
						c2 = texture2DRect( texUnit2, pos.st );\
						c3 = texture2DRect( texUnit3, pos.st );\
						c4 = texture2DRect( texUnit4, pos.st );\
						\
						fc1 = ( c1 * weights.x ) + ( c2 * weights.y ) + ( c3 * weights.z ) + ( c4 * weights.w );\
						\
						c1 = texture2DRect( texUnit5, pos.st );\
						c2 = texture2DRect( texUnit6, pos.st );\
						c3 = texture2DRect( texUnit7, pos.st );\
						c4 = texture2DRect( texUnit8, pos.st );\
						\
						fc2 = ( c1 * weights.x ) + ( c2 * weights.y ) + ( c3 * weights.z ) + ( c4 * weights.w );\
						\
						gl_FragColor = mix(fc1, fc2, transPct / 100.0); \
						gl_FragColor.a = newalpha;\
					}";

				//gl_FragColor = ( c0 * weights.x ) + ( c1 * weights.y ) + ( c2 * weights.z ) + ( c3 * weights.w );
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
					sampler2D texUnit1: register(s1);\
					sampler2D texUnit2: register(s2);\
					sampler2D texUnit3: register(s3);\
					sampler2D texUnit4: register(s4);\
					sampler2D texUnit5: register(s5);\
					sampler2D texUnit6: register(s6);\
					sampler2D texUnit7: register(s7);\
					sampler2D texUnit8: register(s8);\
					float4	weights;\
					float newalpha;\
					float	transPct;\
					float4 main( float2 _uv : TEXCOORD0 ) : COLOR0\
					{\
						float4 c1 = tex2D( texUnit1, _uv );\
						float4 c2 = tex2D( texUnit2, _uv );\
						float4 c3 = tex2D( texUnit3, _uv );\
						float4 c4 = tex2D( texUnit4, _uv );\
						float4 c5 = ( c1 * weights.x ) + ( c2 * weights.y ) + ( c3 * weights.z ) + ( c4 * weights.w );\
						c1 = tex2D( texUnit5, _uv );\
						c2 = tex2D( texUnit6, _uv );\
						c3 = tex2D( texUnit7, _uv );\
						c4 = tex2D( texUnit8, _uv );\
						float4 c6 = ( c1 * weights.x ) + ( c2 * weights.y ) + ( c3 * weights.z ) + ( c4 * weights.w );\
						float4 c7 = lerp( c5, c6, transPct / 100.0 );\
						c7.a = newalpha;\
						return c7;\
					}";


				//	Compile the shader.
				if( _spRenderer->Type() == DisplayOutput::eDX9 )
					m_spShader = _spRenderer->NewShader( cubic_vertexshader, cubic_fragmentshaderDX );
				else
					m_spShader = _spRenderer->NewShader( NULL, ( _spRenderer->GetTextureTargetType() == DisplayOutput::eTexture2DRect ) ? cubic_fragmentshaderGL2DRect : cubic_fragmentshaderGL2D );

				if( !m_spShader )
					m_bValid = false;

				m_NumFrames = 0;
				m_NumSecondFrames = 0;

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
			virtual bool	Update( ContentDecoder::spCContentDecoder _spDecoder, const fp8 _decodeFps, const fp8 /*_displayFps*/, ContentDecoder::sMetaData &_metadata )
			{
				fp4 currentalpha = m_LastAlpha;
				bool frameGrabbed = false;
				bool isSeam = false;
				
				if (m_bWaitNextFrame)
				{
#if !defined(WIN32) && !defined(_MSC_VER)
					if ( !GrabFrame( _spDecoder, m_spFrames[ m_Frames[3] ], m_spFrames[ m_Frames[3] + kMaxFrames ], _metadata) )
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
					if ( GrabFrame( _spDecoder, m_spFrames[ m_Frames[3] ], m_spFrames[ m_Frames[3] + kMaxFrames ], _metadata ) )
					{
						frameGrabbed = true;

						Reset();
						
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
						if( !GrabFrame( _spDecoder, m_spFrames[ m_Frames[3] ], m_spFrames[ m_Frames[3] + kMaxFrames ], _metadata ) )
						{
							m_bWaitNextFrame = true;
#if !defined(WIN32) && !defined(_MSC_VER)
							return false;
#else
							m_NumFrames--;
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
						
					m_NumFrames++;
						
					if (m_spFrames[ m_Frames[3] + kMaxFrames ].IsNull())
						m_NumSecondFrames = 0;
					else
						m_NumSecondFrames++;
						
					isSeam = _metadata.m_IsSeam;
				}


				if (m_NumFrames > 0 && !m_spFrames[ m_Frames[3] ].IsNull())
				{
					//	Enable the shader.
					m_spRenderer->SetShader( m_spShader );
					
					if (isSeam)
					{						
						m_spFrames[ m_Frames[0] ] = m_spFrames[ m_Frames[0] + kMaxFrames ];
						m_spFrames[ m_Frames[1] ] = m_spFrames[ m_Frames[1] + kMaxFrames ];
						m_spFrames[ m_Frames[2] ] = m_spFrames[ m_Frames[2] + kMaxFrames ];
												
						m_spFrames[ m_Frames[0] + kMaxFrames ] = NULL;
						m_spFrames[ m_Frames[1] + kMaxFrames ] = NULL;
						m_spFrames[ m_Frames[2] + kMaxFrames ] = NULL;
					}
					
					uint32 framesToUse = m_NumFrames;
					
					if (framesToUse > kMaxFrames)
						framesToUse = kMaxFrames;
					
					uint32 i;
					
					for (i = 0; i < kMaxFrames-framesToUse; i++)
					{
						uint32 realIdx = m_Frames[ kMaxFrames-framesToUse ];
						
						m_spRenderer->SetTexture( m_spFrames[ realIdx ], i + 1);						
					}
					
					for (i = kMaxFrames-framesToUse; i < kMaxFrames; i++)
					{
						uint32 realIdx = m_Frames[i];

						m_spRenderer->SetTexture( m_spFrames[ realIdx ], i + 1);
					}
					
					if ( m_NumSecondFrames > 0 && !m_spFrames[ m_Frames[3] + kMaxFrames ].IsNull() )
					{
					
						uint32 secFrameToUse = m_NumSecondFrames;
					
						if (secFrameToUse > kMaxFrames)
							secFrameToUse = kMaxFrames;
												
						for (i = 0; i < kMaxFrames-secFrameToUse; i++)
						{
							uint32 realIdx = m_Frames[ kMaxFrames-secFrameToUse ];
														
							m_spRenderer->SetTexture( m_spFrames[ realIdx + kMaxFrames ], i + kMaxFrames + 1);
						}
						
						for (i = kMaxFrames-secFrameToUse; i < kMaxFrames; i++)
						{
							uint32 realIdx = m_Frames[i];
							
							m_spRenderer->SetTexture( m_spFrames[ realIdx + kMaxFrames ], i + kMaxFrames + 1);
						}
					}

					//	B = 1,   C = 0   - cubic B-spline
					//	B = 1/3, C = 1/3 - nice
					//	B = 0,   C = 1/2 - Catmull-Rom spline.
					const fp4 B = 1.0f;
					const fp4 C = 0.0f;

					//	Set the filter weights...
					m_spShader->Set( "weights", MitchellNetravali( fp4(m_InterframeDelta) + 1.f, B, C ), MitchellNetravali( fp4(m_InterframeDelta), B, C ),
												MitchellNetravali( 1.f - fp4(m_InterframeDelta), B, C ), MitchellNetravali( 2.f - fp4(m_InterframeDelta), B, C ) );
					m_spShader->Set( "newalpha", currentalpha);
					
					m_spShader->Set( "transPct", m_MetaData.m_TransitionProgress);

					m_spRenderer->SetBlend( "alphablend" );
					m_spRenderer->Apply();
					m_spRenderer->DrawQuad( m_Size, Base::Math::CVector4( 1, 1, 1, currentalpha), m_spFrames[ m_Frames[3] ]->GetRect() );
				}

				return true;
			}

			virtual fp8 GetFps( fp8 /*_decodeFps*/, fp8 _displayFps )
			{
				return _displayFps;
			}
};

MakeSmartPointers( CCubicFrameDisplay );

#endif

