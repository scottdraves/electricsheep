#ifndef	_SHADERGL_H_
#define	_SHADERGL_H_

#include "Log.h"
#include "Shader.h"

namespace	DisplayOutput
{

class	CShaderUniformGL : public CShaderUniform
{
	uint32	m_Index;
	uint32	m_Size;
	uint8	*m_pData;
	
#ifdef MAC
	CGLContextObj cgl_ctx;
#endif

	public:
#ifdef MAC
			CShaderUniformGL( CGLContextObj glCtx, const std::string _name, const eUniformType _eType, const uint32 _index = 0, const uint32 _size = 0 )
#else
			CShaderUniformGL( const std::string _name, const eUniformType _eType, const uint32 _index = 0, const uint32 _size = 0 )
#endif
			 : CShaderUniform( _name, _eType ), m_Index( _index ), m_Size( _size )
			{
				m_pData = NULL;
				std::string type = "";
				
#ifdef MAC
				cgl_ctx = glCtx;
#endif

				switch( _eType )
				{
					case	eUniform_Sampler:	type = 	"Sampler";	break;
					case	eUniform_Float:		type = 	"Float";	break;
					case	eUniform_Float2:	type = 	"Float2";	break;
					case	eUniform_Float3:	type = 	"Float2";	break;
					case	eUniform_Float4:	type = 	"Float4";	break;
					case	eUniform_Int:		type = 	"Int";	break;
					case	eUniform_Int2:		type = 	"Int2";	break;
					case	eUniform_Int3:		type = 	"Int3";	break;
					case	eUniform_Int4:		type = 	"Int4";	break;
					case	eUniform_Boolean:	type = 	"Boolean";	break;
					case	eUniform_Boolean2:	type = 	"Boolean2";	break;
					case	eUniform_Boolean3:	type = 	"Boolean2";	break;
					case	eUniform_Boolean4:	type = 	"Boolean4";	break;
					case	eUniform_Matrix2:	type = 	"Matrix2";	break;
					case	eUniform_Matrix3:	type = 	"Matrix3";	break;
					case	eUniform_Matrix4:	type = 	"Matrix4";	break;
                    default:
                        type = "";
				}

				if( type == "" )
					g_Log->Warning( ("Unknown uniform " + _name).c_str() );
				else
					g_Log->Info( ("Uniform '" + _name + "' (" + type + ")").c_str() );
			};

			virtual ~CShaderUniformGL()
			{
				SAFE_DELETE_ARRAY( m_pData );
			};

			virtual bool	SetData( void *_pData, const uint32 _size );
			virtual	void	Apply();
};

MakeSmartPointers( CShaderUniformGL );

/*
	CShaderGL().

*/
class CShaderGL : public CShader
{
	GLhandleARB	m_Program;
	GLhandleARB m_VertexShader;
	GLhandleARB m_FragmentShader;
	
#ifdef MAC
	CGLContextObj cgl_ctx;
#endif

	public:
#ifdef MAC
			CShaderGL(CGLContextObj glCtx);
#else
			CShaderGL();
#endif
			virtual ~CShaderGL();

			virtual	bool	Bind( void );
			virtual	bool	Unbind( void );
			virtual bool 	Apply( void );

			bool Build( const char *_pVertexShader, const char *_pFragmentShader );
};

MakeSmartPointers( CShaderGL );

}

#endif
