#include <iostream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#ifndef LINUX_GNU
#include	"./OpenGL/GLee.h"
#include <OpenGL/CGLMacro.h>
#else
#include<GLee.h>
#endif

#include	"Exception.h"
#include	"RendererGL.h"
#include	"ShaderGL.h"
#include	"Log.h"

namespace DisplayOutput
{

/*
*/
#ifdef MAC
CShaderGL::CShaderGL(CGLContextObj glCtx)
#else
CShaderGL::CShaderGL()
#endif
{
	m_VertexShader = 0;
	m_FragmentShader = 0;
	m_Program = 0;
#ifdef MAC
	cgl_ctx = glCtx;//CGLGetCurrentContext();
#endif
}

/*
*/
CShaderGL::~CShaderGL()
{
	if( m_VertexShader )
		glDeleteObjectARB( m_VertexShader );

	if( m_FragmentShader )
		glDeleteObjectARB( m_FragmentShader );

	if( m_Program )
		glDeleteObjectARB( m_Program );
}


/*
*/
bool	CShaderGL::Bind()
{
	glUseProgramObjectARB( m_Program );
	VERIFYGL;
	
	return true;
}

/*
*/
bool	CShaderGL::Apply()
{
	std::map< std::string, spCShaderUniform >::const_iterator	iter;

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
bool	CShaderGL::Unbind()
{
	glUseProgramObjectARB( 0 );
	
	return true;
}

bool	CShaderGL::Build( const char *_pVertexShader, const char *_pFragmentShader )
{
	if( !_pVertexShader && !_pFragmentShader )
		return false;

	const GLcharARB *shaderStrings[6];
	int strIndex = 0;
	char line[16];
	GLint vsResult, fsResult, linkResult;
	char infoLog[2048];
	GLint len, infoLogPos = 0;

	//	Compile to the highest supported language version.
	if( GLEE_ARB_shading_language_100 )
	{
		static char versionString[ 16 ];
		static bool bFirst = true;

		if( bFirst )
		{
//strangely enough on OSX 10.4 GL_ARB_shading_language_100 is defined as 1 but GL_SHADING_LANGUAGE_VERSION_ARB is not defined at all
#if GL_ARB_shading_language_100 && !defined(GL_SHADING_LANGUAGE_VERSION_ARB)
#define GL_SHADING_LANGUAGE_VERSION_ARB                    0x8B8C
#endif
			const char *pVersion = (const char *)glGetString( GL_SHADING_LANGUAGE_VERSION_ARB );
			if( pVersion )
			{
				snprintf( versionString, 16, "#version %d%d\n", atoi( pVersion ), atoi( strchr( pVersion, '.') + 1) );
				g_Log->Info( "GL shader %s", versionString );
			}

			bFirst = false;
		}

		shaderStrings[ strIndex++ ] = versionString;
	}

	shaderStrings[strIndex++] = line;

	m_VertexShader = 0;
	m_FragmentShader = 0;
	m_Program = glCreateProgramObjectARB();

	//	Compile the vertex shader.
	if( _pVertexShader != NULL )
	{
		m_VertexShader = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );
		snprintf(line, 16, "#line %d\n", 0 );
		shaderStrings[ strIndex ] = _pVertexShader;
		glShaderSourceARB( m_VertexShader, strIndex + 1, shaderStrings, NULL );
		glCompileShaderARB( m_VertexShader );
		glGetObjectParameterivARB( m_VertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &vsResult );
		if( vsResult )
			glAttachObjectARB( m_Program, m_VertexShader );
		else
			infoLogPos += sprintf( infoLog + infoLogPos, "Vertex shader error:\n" );

		glGetInfoLogARB( m_VertexShader, sizeof(infoLog) - infoLogPos, &len, infoLog + infoLogPos );
		infoLogPos += len;

	}
	else
		vsResult = GL_TRUE;

	//	Compile the fragment shader.
	if( _pFragmentShader != NULL )
	{
		m_FragmentShader = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );
		sprintf( line, "#line %d\n", 0 );
		shaderStrings[ strIndex ] = _pFragmentShader;
		glShaderSourceARB( m_FragmentShader, strIndex + 1, shaderStrings, NULL );
		glCompileShaderARB( m_FragmentShader );
		glGetObjectParameterivARB( m_FragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &fsResult );
		if( fsResult )
			glAttachObjectARB( m_Program, m_FragmentShader );
		else
			infoLogPos += sprintf( infoLog + infoLogPos, "Fragment shader error:\n" );

		glGetInfoLogARB( m_FragmentShader, sizeof(infoLog) - infoLogPos, &len, infoLog + infoLogPos );
		infoLogPos += len;
	}
	else
		fsResult = GL_TRUE;

	//	Link the shaders.
	if( vsResult && fsResult )
	{
		glLinkProgramARB( m_Program );
		glGetObjectParameterivARB( m_Program, GL_OBJECT_LINK_STATUS_ARB, &linkResult );
		glGetInfoLogARB( m_Program, sizeof(infoLog) - infoLogPos, &len, infoLog + infoLogPos );
		infoLogPos += len;

		if( len > 0 )
			g_Log->Warning( infoLog );

		if( linkResult )
		{
			glUseProgramObjectARB( m_Program );

			GLint uniformCount, maxLength;
			glGetObjectParameterivARB( m_Program, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &uniformCount );
			glGetObjectParameterivARB( m_Program, GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, &maxLength );

			int nSamplers = 0;
			//int nUniforms = 0;
			char *name = new char[maxLength];

			for( int i=0; i<uniformCount; i++ )
			{
				GLenum type;
				GLint length, size;
				glGetActiveUniformARB( m_Program, i, maxLength, &length, &size, &type, name );

				if( type >= GL_SAMPLER_1D && type <= GL_SAMPLER_2D_RECT_SHADOW_ARB )
				{
					//	Assign samplers to image units.
					GLint location = glGetUniformLocationARB( m_Program, name );
					
					int pos;
					
					sscanf(name, "texUnit%d", &pos);
					
					glUniform1i( location, pos/*nSamplers*/ );
#ifdef MAC
					m_Samplers[ name ] = new CShaderUniformGL( cgl_ctx, name, eUniform_Sampler, location );
#else
					m_Samplers[ name ] = new CShaderUniformGL( name, eUniform_Sampler, location );
#endif
					nSamplers++;
				}
				else
				{
					//	Store all non-gl uniforms.
					if( strncmp(name, "gl_", 3) != 0 )
					{
						char *bracket = strchr(name, '[');
						if( bracket == NULL || ( bracket[1] == '0' && bracket[2] == ']' ) )
						{
							if( bracket )
							{
								*bracket = '\0';
								length = (GLint) (bracket - name);
							}

							eUniformType eType = eUniform_Float;

							switch( type )
							{
								case GL_FLOAT:          eType = eUniform_Float;		break;
								case GL_FLOAT_VEC2_ARB: eType = eUniform_Float2;	break;
								case GL_FLOAT_VEC3_ARB: eType = eUniform_Float3;	break;
								case GL_FLOAT_VEC4_ARB: eType = eUniform_Float4;	break;
								case GL_INT:            eType = eUniform_Int;		break;
								case GL_INT_VEC2_ARB:   eType = eUniform_Int2;		break;
								case GL_INT_VEC3_ARB:   eType = eUniform_Int3;		break;
								case GL_INT_VEC4_ARB:   eType = eUniform_Int4;		break;
								case GL_BOOL_ARB:       eType = eUniform_Boolean;	break;
								case GL_BOOL_VEC2_ARB:  eType = eUniform_Boolean2;	break;
								case GL_BOOL_VEC3_ARB:  eType = eUniform_Boolean3;	break;
								case GL_BOOL_VEC4_ARB:  eType = eUniform_Boolean4;	break;
								case GL_FLOAT_MAT2_ARB: eType = eUniform_Matrix2;	break;
								case GL_FLOAT_MAT3_ARB: eType = eUniform_Matrix3;	break;
								case GL_FLOAT_MAT4_ARB: eType = eUniform_Matrix4;	break;
							}
						
#ifdef MAC
							m_Uniforms[ name ] = new CShaderUniformGL( cgl_ctx, name, eType, glGetUniformLocationARB( m_Program, name ), size );
#else
							m_Uniforms[ name ] = new CShaderUniformGL( name, eType, glGetUniformLocationARB( m_Program, name ), size );
#endif
						}
						else if( bracket != NULL && bracket[1] > '0' )
						{
							/**bracket = '\0';
							for( int i = nUniforms - 1; i >= 0; i--)
							{
								if( strcmp( uniforms[i].name, name ) == 0 )
								{
									int index = atoi(bracket + 1) + 1;
									if( index > uniforms[i].nElements )
										uniforms[i].nElements = index;
								}
							}*/
						}
					}
				}
			}

			SAFE_DELETE_ARRAY( name );

/*			for (int i = 0; i < nUniforms; i++)
			{
				int constantSize = constantTypeSizes[uniforms[i].type] * uniforms[i].nElements;
				uniforms[i].data = new ubyte[constantSize];
				memset(uniforms[i].data, 0, constantSize);
				uniforms[i].dirty = false;
			}
			shader.uniforms  = uniforms;
			shader.samplers  = samplers;
			shader.nUniforms = nUniforms;
			shader.nSamplers = nSamplers;

			return shaders.add(shader);*/

			glUseProgram( GL_NONE );

			VERIFYGL;

			return true;
		}
	}

	g_Log->Error( infoLog );
	return false;
}

//	Static list of pointers to opengl uniform functions.
static void *g_UniformFunctionList[ eUniform_NumUniformTypes-1 ];
static bool	g_bFunclistValid = false;

/*
*/
bool	CShaderUniformGL::SetData( void *_pData, const uint32 _size )
{
	//	Let's take this opportunity to make sure the function pointers are correctly set up.. Ugly, fix!
	if( !g_bFunclistValid )
	{
		g_UniformFunctionList[ eUniform_Float ] = (void *)glUniform1fvARB;
		g_UniformFunctionList[ eUniform_Float2 ] = (void *)glUniform2fvARB;
		g_UniformFunctionList[ eUniform_Float3 ] = (void *)glUniform3fvARB;
		g_UniformFunctionList[ eUniform_Float4 ] = (void *)glUniform4fvARB;
		g_UniformFunctionList[ eUniform_Int ] = (void *)glUniform1ivARB;
		g_UniformFunctionList[ eUniform_Int2 ] = (void *)glUniform2ivARB;
		g_UniformFunctionList[ eUniform_Int3 ] = (void *)glUniform3ivARB;
		g_UniformFunctionList[ eUniform_Int4 ] = (void *)glUniform4ivARB;
		g_UniformFunctionList[ eUniform_Boolean ] = (void *)glUniform1ivARB;
		g_UniformFunctionList[ eUniform_Boolean2 ] = (void *)glUniform2ivARB;
		g_UniformFunctionList[ eUniform_Boolean3 ] = (void *)glUniform3ivARB;
		g_UniformFunctionList[ eUniform_Boolean4 ] = (void *)glUniform4ivARB;
		g_UniformFunctionList[ eUniform_Matrix2 ] = (void *)glUniformMatrix2fvARB;
		g_UniformFunctionList[ eUniform_Matrix3 ] = (void *)glUniformMatrix3fvARB;
		g_UniformFunctionList[ eUniform_Matrix4 ] = (void *)glUniformMatrix4fvARB;
		g_bFunclistValid = true;
	}

	if( m_pData == NULL )
	{
		uint32 size = 0;

		switch( m_eType )
		{
			case	eUniform_Sampler:	size = sizeof(int32);		break;
			case	eUniform_Float:		size = sizeof(fp4);			break;
			case	eUniform_Float2:	size = sizeof(fp4)*2;		break;
			case	eUniform_Float3:	size = sizeof(fp4)*2;		break;
			case	eUniform_Float4:	size = sizeof(fp4)*4;		break;
			case	eUniform_Int:		size = sizeof(int32);		break;
			case	eUniform_Int2:		size = sizeof(int32)*2;		break;
			case	eUniform_Int3:		size = sizeof(int32)*3;		break;
			case	eUniform_Int4:		size = sizeof(int32)*4;		break;
			case	eUniform_Boolean:	size = sizeof(int32);		break;
			case	eUniform_Boolean2:	size = sizeof(int32)*2;		break;
			case	eUniform_Boolean3:	size = sizeof(int32)*2;		break;
			case	eUniform_Boolean4:	size = sizeof(int32)*4;		break;
			case	eUniform_Matrix2:	size = sizeof(fp4)*(4*2);	break;
			case	eUniform_Matrix3:	size = sizeof(fp4)*(4*3);	break;
			case	eUniform_Matrix4:	size = sizeof(fp4)*(4*4);	break;
		}

		if( _size != size )
			g_Log->Warning( "hmm, uniform size != _size?");

		m_pData = new uint8[ size ];
	}

	if( memcmp( m_pData, _pData, _size ) )
	{
		memcpy( m_pData, _pData, _size );
		m_bDirty = true;
	}

	return true;
}

typedef GLvoid (APIENTRY *Uniform_Func)(GLint location, GLsizei count, const void *value);
typedef GLvoid (APIENTRY *Uniform_MatrixFunc)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

/*
*/
void	CShaderUniformGL::Apply()
{
	if( !m_bDirty || m_pData == NULL )
		return;
		
	switch (m_eType)
	{
		case eUniform_Float:
			glUniform1fvARB( m_Index, m_Size, (const GLfloat *)m_pData );
			break;
		case eUniform_Float2:
			glUniform2fvARB( m_Index, m_Size, (const GLfloat *)m_pData );
			break;
		case eUniform_Float3:
			glUniform3fvARB( m_Index, m_Size, (const GLfloat *)m_pData );
			break;
		case eUniform_Float4:
			glUniform4fvARB( m_Index, m_Size, (const GLfloat *)m_pData );
			break;
		case eUniform_Int:
		case eUniform_Boolean:
		case eUniform_Sampler:
			glUniform1ivARB( m_Index, m_Size, (const GLint *)m_pData );
			break;
		case eUniform_Int2:
		case eUniform_Boolean2:
			glUniform2ivARB( m_Index, m_Size, (const GLint *)m_pData );
			break;
		case eUniform_Int3:
		case eUniform_Boolean3:
			glUniform3ivARB( m_Index, m_Size, (const GLint *)m_pData );
			break;
		case eUniform_Int4:
		case eUniform_Boolean4:
			glUniform4ivARB( m_Index, m_Size, (const GLint *)m_pData );
			break;
		case eUniform_Matrix2:
			glUniformMatrix2fvARB( m_Index, m_Size, GL_TRUE, (const GLfloat *)m_pData );
			break;
		case eUniform_Matrix3:
			glUniformMatrix3fvARB( m_Index, m_Size, GL_TRUE, (const GLfloat *)m_pData );
			break;
		case eUniform_Matrix4:
			glUniformMatrix4fvARB( m_Index, m_Size, GL_TRUE, (const GLfloat *)m_pData );
			break;
	}

	VERIFYGL;
	m_bDirty = false;
}

}

