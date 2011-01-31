#ifndef	_SHADERDX_H_
#define	_SHADERDX_H_

#include "Log.h"
#include "Shader.h"

namespace	DisplayOutput
{

class	CShaderUniformDX : public CShaderUniform
{
	uint32	m_Index;
	uint32	m_Size;
	uint8	*m_pData;
	fp4		m_float4Data[4];

	IDirect3DDevice9	*m_pDevice;

	public:
			CShaderUniformDX( IDirect3DDevice9	*_pDevice, const std::string _name, const eUniformType _eType, const uint32 _index = 0, const uint32 _size = 0 ) :
		  CShaderUniform( _name, _eType ), m_Index( _index ), m_pDevice( _pDevice ), m_pData(NULL)
			{
				std::string type;

				switch( _eType )
				{
					case	eUniform_Sampler:	type = 	"Sampler";	break;
					case	eUniform_Float:		type = 	"Float";	break;
					case	eUniform_Float2:	type = 	"Float2";	break;
					case	eUniform_Float3:	type = 	"Float2";	break;
					case	eUniform_Float4:	type = 	"Float4";	break;
					case	eUniform_Int:		type = 	"Int";		break;
					case	eUniform_Int2:		type = 	"Int2";		break;
					case	eUniform_Int3:		type = 	"Int3";		break;
					case	eUniform_Int4:		type = 	"Int4";		break;
					case	eUniform_Boolean:	type = 	"Boolean";	break;
					case	eUniform_Boolean2:	type = 	"Boolean2";	break;
					case	eUniform_Boolean3:	type = 	"Boolean2";	break;
					case	eUniform_Boolean4:	type = 	"Boolean4";	break;
					case	eUniform_Matrix2:	type = 	"Matrix2";	break;
					case	eUniform_Matrix3:	type = 	"Matrix3";	break;
					case	eUniform_Matrix4:	type = 	"Matrix4";	break;
				}

				if( type.size() == 0 )
					g_Log->Warning( ("Unknown uniform " + _name).c_str() );
				else
					g_Log->Info( ("Uniform '" + _name + "' (" + type + ")").c_str() );
			};

			virtual ~CShaderUniformDX()
			{
				SAFE_DELETE_ARRAY( m_pData );
			};

			virtual bool	SetData( void *_pData, const uint32 _size );
			virtual	void	Apply();
};

MakeSmartPointers( CShaderUniformDX );

/*
	CShaderDX().

*/
class CShaderDX : public CShader
{
	IDirect3DDevice9	*m_pDevice;

	IDirect3DVertexShader9	*m_pVertexShader;
	IDirect3DPixelShader9	*m_pFragmentShader;

	ID3DXConstantTable		*m_pVertexConstants;
	ID3DXConstantTable		*m_pFragmentConstants;

	float m_Width;
	float m_Height;

	public:
			CShaderDX( IDirect3DDevice9	*_pDevice, float _width, float _height );
			virtual ~CShaderDX();

			virtual	bool	Bind( void );
			virtual	bool	Unbind( void );
			virtual bool 	Apply( void );

			bool Build( const char *_pVertexShader, const char *_pFragmentShader );
};

MakeSmartPointers( CShaderDX );

}

#endif
