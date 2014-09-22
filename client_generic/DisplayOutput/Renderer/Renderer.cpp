#include	<stdint.h>
#include	<string.h>

#include	"DisplayOutput.h"
#include	"Renderer.h"
#include	"Matrix.h"

namespace DisplayOutput
{

/*
*/
CRenderer::CRenderer()
{
	m_aspActiveTextures = new spCTexture[ MAX_TEXUNIT ];
	m_aspSelectedTextures = new spCTexture[ MAX_TEXUNIT ];

	m_bDirtyMatrices = 0;
}

/*
*/
CRenderer::~CRenderer()
{
	if (m_aspSelectedTextures != NULL)
	for( uint32 i=0; i<MAX_TEXUNIT; i++ )
		m_aspSelectedTextures[ i ] = NULL;
	if (m_aspActiveTextures != NULL)
	for( uint32 i=0; i<MAX_TEXUNIT; i++ )
		m_aspActiveTextures[ i ] = NULL;
	SAFE_DELETE_ARRAY(m_aspActiveTextures);
	SAFE_DELETE_ARRAY(m_aspSelectedTextures);
}

/*
*/
bool	CRenderer::Initialize( spCDisplayOutput _spDisplay )
{
    m_spDisplay = _spDisplay;

	//	Default(disabled) blendmode.
    AddBlend( "none", eOne, eZero, eAdd );

    //	Alphablend.
    AddBlend( "alphablend", eSrc_Alpha, eOne_Minus_Src_Alpha, eAdd );
	return true;
}

/*
*/
void	CRenderer::AddBlend( std::string _name, int32 _src, int32 _dst, int32 _mode )
{
	spCBlend spBlend = new CBlend( _src, _dst, _mode );
	m_BlendMap[ _name ] = spBlend;
}

/*
*/
void	CRenderer::Reset( const uint32 _flags )
{
	if( isBit( _flags, eShader ) )
		m_spSelectedShader = NULL;

	if( isBit( _flags, eBlend ) )
		m_spSelectedBlend = m_BlendMap[ "none" ];

	if( isBit( _flags, eTexture ) )
	{
		for( uint32 i=0; i<MAX_TEXUNIT; i++ )
			m_aspSelectedTextures[ i ] = NULL;
	}

	if( isBit( _flags, eMatrices ) )
	{
		m_WorldMat.Identity();
		m_ViewMat.Identity();
		m_ProjMat.Identity();
		setBit( m_bDirtyMatrices, eWorld );
		setBit( m_bDirtyMatrices, eView );
		setBit( m_bDirtyMatrices, eProjection );
	}
}


/*
*/
void	CRenderer::Orthographic()
{
	Orthographic( m_spDisplay->Width(), m_spDisplay->Height() );
}

/*
*/
void	CRenderer::Orthographic( const uint32 _width, const uint32 _height )
{
	Base::Math::CMatrix4x4 proj;
	proj.OrthographicRH( fp4(_width), fp4(_height), -1, 1 );
	SetTransform( proj, eProjection );
}

/*
*/
void	CRenderer::SetTransform( const Base::Math::CMatrix4x4 &_transform, const eMatrixTransformType _type )
{
	switch( _type )
	{
		case	eWorld:			m_WorldMat = _transform;	break;
		case	eView:			m_ViewMat = _transform;		break;
		case	eProjection:	m_ProjMat = _transform;		break;
		default:
				g_Log->Warning( "Unknown transformation type..." );
	}

	setBit( m_bDirtyMatrices, _type );
}


/*
	SetTexture().

*/
void	CRenderer::SetTexture( spCTexture _spTex, const uint32 _index )
{
	ASSERT( _index <=MAX_TEXUNIT );
	m_aspSelectedTextures[ _index ] = _spTex;
}

/*
	SetShader().
*/
void	CRenderer::SetShader( spCShader _spShader )
{
	m_spSelectedShader = _spShader;
}

/*
*/
void 	CRenderer::SetBlend( std::string _blend )
{
	m_spSelectedBlend = m_BlendMap[ _blend ];
}


/*
*/
void	CRenderer::Apply()
{
	//	Update textures.
	for( uint32 i=0; i<MAX_TEXUNIT; i++ )
	{
		spCTexture	spTex = m_aspSelectedTextures[i];
#if 0
#warning FIXME (Keffo#1#): hum, same pointer, different tex?
#endif
		if (spTex.IsNull())
		{
			if (!m_aspActiveTextures[i].IsNull())
			{
				m_aspActiveTextures[i]->Unbind( i );
				m_aspActiveTextures[i] = NULL;
			}
		}
		else
		if( spTex != m_aspActiveTextures[i] )
		{
			if( m_aspActiveTextures[i] != NULL )
				m_aspActiveTextures[i]->Unbind( i );

			if( spTex != NULL )
			{
				//	Bind new tex.
				spTex->Bind( i );
			}

			m_aspActiveTextures[ i ] = spTex;
		}

		//	Force bind if dirty.
		if( spTex != NULL )
			if( spTex->Dirty() )
			{
				spTex->Bind( i );
				m_aspActiveTextures[ i ] = spTex;
			}

	}

	//	Update shader.
	if( m_spActiveShader != m_spSelectedShader )
	{
		if( m_spActiveShader != NULL )
			m_spActiveShader->Unbind();

		if( m_spSelectedShader != NULL )
			m_spSelectedShader->Bind();

		m_spActiveShader = m_spSelectedShader;
	}

	//	Update shader uniforms.
	if( m_spActiveShader != NULL )
		m_spActiveShader->Apply();
}


}

