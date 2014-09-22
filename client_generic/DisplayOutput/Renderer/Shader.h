#ifndef	_SHADER_H_
#define	_SHADER_H_

#include <map>
#include "Log.h"
#include "SmartPtr.h"

namespace	DisplayOutput
{

enum	eUniformType	{
	eUniform_Float = 0,
	eUniform_Float2,
	eUniform_Float3,
	eUniform_Float4,
	eUniform_Int,
	eUniform_Int2,
	eUniform_Int3,
	eUniform_Int4,
	eUniform_Boolean,
	eUniform_Boolean2,
	eUniform_Boolean3,
	eUniform_Boolean4,
	eUniform_Matrix2,
	eUniform_Matrix3,
	eUniform_Matrix4,
	eUniform_Sampler,
	eUniform_NumUniformTypes,
};


/*
	CShaderUniform.

*/
class	CShaderUniform
{
	protected:
		bool	m_bDirty;
		std::string		m_Name;
		eUniformType	m_eType;

	public:
			CShaderUniform( const std::string _name, const eUniformType _eType ) :  m_bDirty(true), m_Name( _name ), m_eType( _eType )	{}
			virtual ~CShaderUniform()	{}

			virtual bool	SetData( void *_pData, const uint32 _size ) = PureVirtual;
			virtual void	Apply() = PureVirtual;
};

MakeSmartPointers( CShaderUniform );


/*
	CShader().

*/
class CShader
{
	protected:
		std::map< std::string, spCShaderUniform > m_Uniforms;
		std::map< std::string, spCShaderUniform > m_Samplers;

		spCShaderUniform	Uniform( const std::string _name ) const
		{
			spCShaderUniform ret = NULL;

			std::map< std::string, spCShaderUniform >::const_iterator iter;

			iter = m_Uniforms.find( _name );
			if( iter != m_Uniforms.end() )
				return iter->second;

			iter = m_Samplers.find( _name );
			if( iter != m_Samplers.end() )
				return iter->second;

			//ThrowStr( ("Uniform '" + _name + "' not found").c_str() );
			//g_Log->Warning( "Uniform '%s' not found", _name.c_str() );
			return NULL;
		}

	public:
			CShader();
			virtual ~CShader();

			virtual	bool	Bind( void ) = PureVirtual;
			virtual	bool	Unbind( void ) = PureVirtual;
			virtual bool 	Apply( void ) = PureVirtual;

			virtual bool	Build( const char *_pVertexShader, const char *_pFragmentShader ) = PureVirtual;

			bool	Set( const std::string _name, const int32 _value ) const
			{
				spCShaderUniform spUniform = Uniform( _name );
				if( spUniform != NULL )
				{
					spUniform->SetData( (void *)&_value, sizeof(_value) );
					return true;
				}

				return false;
			}

			bool	Set( const std::string _name, const fp4 _value ) const
			{
				spCShaderUniform spUniform = Uniform( _name );
				if( spUniform != NULL )
				{
					spUniform->SetData( (void *)&_value, sizeof(_value) );
					return true;
				}

				return false;
			}

			bool	Set( const std::string _name, const fp4 _x, const fp4 _y, const fp4 _z, const fp4 _w ) const
			{
				spCShaderUniform spUniform = Uniform( _name );
				if( spUniform != NULL )
				{
					fp4 v[4] = { _x, _y, _z, _w };
					spUniform->SetData( &v, sizeof(v) );
					return true;
				}

				return false;
			}
};

MakeSmartPointers( CShader );

}

#endif
