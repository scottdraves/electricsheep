#include <iostream>
#include <assert.h>

#if defined(WIN32) && defined(_MSC_VER)
#include	"d3dx9.h"
#include	"../msvc/msvc_fix.h"
#endif
#include	"Exception.h"
#include	"RendererDX.h"
#include	"ShaderDX.h"
#include	"Matrix.h"
#include	"Log.h"

namespace DisplayOutput
{

/*
*/
CShaderDX::CShaderDX( IDirect3DDevice9 *_pDevice, float _width, float _height ) : m_pDevice( _pDevice ), m_Width(_width), m_Height(_height)
{
	if (_pDevice == NULL)
		g_Log->Error( "CShaderDX received device == NULL" );
	m_pVertexShader = NULL;
	m_pFragmentShader = NULL;
}

/*
*/
CShaderDX::~CShaderDX()
{
	SAFE_RELEASE( m_pVertexShader );
	SAFE_RELEASE( m_pFragmentShader );
}


/*
*/
bool	CShaderDX::Bind()
{
	m_pDevice->SetVertexShader( m_pVertexShader );
	m_pDevice->SetPixelShader( m_pFragmentShader );
	return true;
}

/*
*/
bool	CShaderDX::Apply()
{
	std::map< std::string, spCShaderUniform >::const_iterator	iter;

	if( m_pVertexShader )
	{
		spCShaderUniform	spUni = Uniform( "WorldViewProj" );
		if( spUni != NULL )
		{
			Base::Math::CMatrix4x4 matWorld, matView, matProj, matWorldViewProj;

			matWorld.Identity();
			matView.Identity();
			matProj.OrthographicRH(m_Width, m_Height, -1.f, 1.f);

			matWorldViewProj = matWorld * matView * matProj;

			spUni->SetData( (D3DMATRIX *) (const fp4 *)matWorldViewProj.m_Mat, sizeof(D3DXMATRIX) );
		}
	}


	//	Update all uniforms.
	for( iter = m_Uniforms.begin(); iter != m_Uniforms.end(); ++iter )
		((spCShaderUniform)iter->second)->Apply();

	//	Update all samplers.
	for( iter = m_Samplers.begin(); iter != m_Samplers.end(); ++iter )
		((spCShaderUniform)iter->second)->Apply();
	return true;
}

/*
*/
bool	CShaderDX::Unbind()
{
	m_pDevice->SetVertexShader( NULL );
	m_pDevice->SetPixelShader( NULL );
	return true;
}

/*
*/
bool	CShaderDX::Build( const char *_pVertexShader, const char *_pFragmentShader )
{
	if( !_pVertexShader && !_pFragmentShader )
		return false;

	g_Log->Info( "CShaderDX::Build" );

	LPD3DXBUFFER pShaderBuf = NULL;
	LPD3DXBUFFER pErrorsBuf = NULL;

	if( _pVertexShader != NULL )
	{
		std::string shaderString = _pVertexShader;

		const char *profile = g_DLLFun->D3DXGetVertexShaderProfile_fun( m_pDevice );

		if( g_DLLFun->D3DXCompileShader_fun( shaderString.c_str(), (UINT)shaderString.length(), NULL, NULL, "main", profile, D3DXSHADER_SKIPVALIDATION, &pShaderBuf, &pErrorsBuf, &m_pVertexConstants ) == D3D_OK )
		{
			if (FAILED(m_pDevice->CreateVertexShader( (DWORD *)pShaderBuf->GetBufferPointer(), &m_pVertexShader )))
			{
				g_Log->Error( "Unable to create vertex shader" );
				if (pErrorsBuf != NULL)
					g_Log->Error( "DX Error: %s", (const char *)pErrorsBuf->GetBufferPointer() );
			}
		}
		else
		{
			g_Log->Error( "Unable to compile vertex shaders" );
			if (pErrorsBuf != NULL)
				g_Log->Error( "DX Error: %s", (const char *)pErrorsBuf->GetBufferPointer() );
		}

		SAFE_RELEASE( pShaderBuf );
		SAFE_RELEASE( pErrorsBuf );

		if( !m_pVertexShader )
			return false;
	}

	if( _pFragmentShader != NULL )
	{
		std::string shaderString = _pFragmentShader;

		const char *profile = g_DLLFun->D3DXGetPixelShaderProfile_fun( m_pDevice );

		if( g_DLLFun->D3DXCompileShader_fun( shaderString.c_str(), (UINT)shaderString.length(), NULL, NULL, "main", profile, D3DXSHADER_SKIPVALIDATION, &pShaderBuf, &pErrorsBuf, &m_pFragmentConstants ) == D3D_OK )
		{
			if (FAILED(m_pDevice->CreatePixelShader( (DWORD *)pShaderBuf->GetBufferPointer(), &m_pFragmentShader )))
			{
				g_Log->Error( "Unable to create fragment shader" );
				if (pErrorsBuf != NULL)
					g_Log->Error( (const char *)pErrorsBuf->GetBufferPointer() );
			}
		} else
		{
			g_Log->Error( "Unable to compile fragment shader" );
			if (pErrorsBuf != NULL)
				g_Log->Error( (const char *)pErrorsBuf->GetBufferPointer() );
		}


		SAFE_RELEASE( pShaderBuf );
		SAFE_RELEASE( pErrorsBuf );

		if( !m_pFragmentShader )
			return false;
	}

	D3DXCONSTANT_DESC cDesc;

	if( m_pVertexShader )
	{
		D3DXCONSTANTTABLE_DESC vsDesc;
		m_pVertexConstants->GetDesc( &vsDesc );

		for( uint32 i=0; i<vsDesc.Constants; i++ )
		{
			UINT count = 1;
			m_pVertexConstants->GetConstantDesc( m_pVertexConstants->GetConstant( NULL, i ), &cDesc, &count );

			if( cDesc.Type >= D3DXPT_SAMPLER && cDesc.Type <= D3DXPT_SAMPLERCUBE )
			{
				g_Log->Error( "go vertex sample something else.." );
				return false;
			}

			eUniformType eType = eUniform_Float;

			if( cDesc.Class == D3DXPC_MATRIX_COLUMNS )
			{
				switch( cDesc.Type )
				{
					case	D3DXPT_FLOAT:	eType = eUniform_Matrix4;		break;
					default:
							g_Log->Warning( "Unknown vertex uniform type for %s -> %d", cDesc.Name, cDesc.Type );
				}
			}
			else
			{
				switch( cDesc.Type )
				{
					case	D3DXPT_BOOL:	eType = eUniform_Boolean;	break;
					case	D3DXPT_INT:		eType = eUniform_Int;		break;
					case	D3DXPT_FLOAT:	eType = eUniform_Float;		break;
					default:
							g_Log->Warning( "Unknown vertex uniform type for %s -> %d", cDesc.Name, cDesc.Type );
				}
			}

			m_Uniforms[ cDesc.Name ] = new CShaderUniformDX( m_pDevice, cDesc.Name, eType, cDesc.RegisterIndex );
		}
	}

	if( m_pFragmentShader )
	{
		D3DXCONSTANTTABLE_DESC psDesc;
		m_pFragmentConstants->GetDesc( &psDesc );

		for( uint32 i=0; i<psDesc.Constants; i++ )
		{
			UINT count = 1;
			m_pFragmentConstants->GetConstantDesc( m_pFragmentConstants->GetConstant( NULL, i ), &cDesc, &count );

			if( cDesc.Type >= D3DXPT_SAMPLER && cDesc.Type <= D3DXPT_SAMPLERCUBE )
				m_Samplers[ cDesc.Name ] = new CShaderUniformDX( m_pDevice, cDesc.Name, eUniform_Sampler, cDesc.RegisterIndex );
			else
			{
				//	Already from vs?
				spCShaderUniform	spUni = Uniform( cDesc.Name );
				if( spUni != NULL )
					g_Log->Error( "code this!" );
				else
				{
					eUniformType eType = eUniform_Float;

					if( cDesc.Class == D3DXPC_VECTOR )
					{
						switch( cDesc.Type )
						{
							case	D3DXPT_BOOL:	eType = eUniform_Boolean;	break;
							case	D3DXPT_INT:		eType = eUniform_Int;		break;
							case	D3DXPT_FLOAT:	eType = eUniform_Float4;	break;
							default:
									g_Log->Warning( "Unknown pixel uniform type for %s -> %d", cDesc.Name, cDesc.Type );
						}
					}
					else
					{
						switch( cDesc.Type )
						{
							case	D3DXPT_BOOL:	eType = eUniform_Boolean;	break;
							case	D3DXPT_INT:		eType = eUniform_Int;		break;
							case	D3DXPT_FLOAT:	eType = eUniform_Float;		break;
							default:
									g_Log->Warning( "Unknown pixel uniform type for %s -> %d", cDesc.Name, cDesc.Type );
						}
					}

					m_Uniforms[ cDesc.Name ] = new CShaderUniformDX( m_pDevice, cDesc.Name, eType, cDesc.RegisterIndex );
				}
			}
		}
	}

	return true;
}

/*
*/
bool	CShaderUniformDX::SetData( void *_pData, const uint32 _size )
{
	if( m_pData == NULL )
	{
		m_Size = 0;

		switch( m_eType )
		{
			case	eUniform_Sampler:	m_Size = sizeof(int32);		break;
			case	eUniform_Float:		m_Size = sizeof(fp4);		break;
			case	eUniform_Float4:	m_Size = sizeof(fp4)*4;		break;
			case	eUniform_Int:		m_Size = sizeof(int32);		break;
			case	eUniform_Boolean:	m_Size = sizeof(BOOL);		break;
			case	eUniform_Matrix4:	m_Size = sizeof(fp4)*16;	break;
			default:
					g_Log->Warning( "Unknown uniform type" );
					return false;
					break;
		}

		if( m_Size != _size )
			g_Log->Warning( "hmm, uniform size != _size?");

		m_pData = new uint8[ m_Size ];
		ZeroMemory(m_pData, m_Size);
	}

	if( memcmp( m_pData, _pData, _size ) )
	{
		if (m_eType == eUniform_Float)
		{
			memcpy(m_float4Data, _pData, sizeof(float));
			m_float4Data[1] = 0.f;
			m_float4Data[2] = 0.f;
			m_float4Data[3] = 0.f;
		}
		memcpy( m_pData, _pData, _size );
		m_bDirty = true;
	}

	return true;
}

/*
*/
void	CShaderUniformDX::Apply()
{
	if( !m_bDirty || m_pData == NULL )
		return;

	switch( m_eType )
	{
//		case	eUniform_Sampler:	break;
		case	eUniform_Float: m_pDevice->SetPixelShaderConstantF( m_Index, (const float *)m_float4Data, 1); break;
		case	eUniform_Float4: m_pDevice->SetPixelShaderConstantF( m_Index, (const float *)m_pData, m_Size/sizeof(float)/4 );	break;
		case	eUniform_Int: g_Log->Warning( "eUniform_Int used in ShaderDX"); m_pDevice->SetPixelShaderConstantI( m_Index, (const int32 *)m_pData, m_Size/sizeof(int32)/4 );	break;
		case	eUniform_Boolean:	m_pDevice->SetPixelShaderConstantB( m_Index, (const BOOL *)m_pData, m_Size/sizeof(BOOL));	break;
		case	eUniform_Matrix4:	m_pDevice->SetVertexShaderConstantF( m_Index, (const fp4 *)m_pData, m_Size/sizeof(float)/4  );	break;
	}

	// Always update everything
	// Uncommenting this will break shaders after device reset and few other situations
	//m_bDirty = false;
}

}

