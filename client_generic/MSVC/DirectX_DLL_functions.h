#pragma once
#include	"base.h"
#include	"log.h"
#include	"SmartPtr.h"
#include	"Singleton.h"
#include <d3d9.h>
#include <d3dx9.h>

namespace Base
{
MakeSmartPointers( DXDLLFun );

class	DXDLLFun : public CSingleton<DXDLLFun>
{
	friend class CSingleton<DXDLLFun>;

	//	Private constructor accessible only to CSingleton.
	DXDLLFun():d3dx9_hmodule(NULL), D3DXCompileShader_fun(NULL), D3DXGetVertexShaderProfile_fun(NULL),
		D3DXCreateFontA_fun(NULL), D3DXCreateSprite_fun(NULL), D3DXMatrixMultiply_fun(NULL),
		D3DXGetPixelShaderProfile_fun(NULL), m_bInitDone(false)
	{
	}
	virtual ~DXDLLFun() 
	{
	}

	//	No copy constructor or assignment operator.
    NO_CLASS_STANDARDS( DXDLLFun );

	typedef HRESULT (WINAPI *D3DXCreateFontA_fun_type)(
        LPDIRECT3DDEVICE9       pDevice,  
        INT                     Height,
        UINT                    Width,
        UINT                    Weight,
        UINT                    MipLevels,
        BOOL                    Italic,
        DWORD                   CharSet,
        DWORD                   OutputPrecision,
        DWORD                   Quality,
        DWORD                   PitchAndFamily,
        LPCSTR                  pFaceName,
        LPD3DXFONT*             ppFont) ;

	typedef HRESULT (WINAPI *D3DXCreateSprite_fun_type)( 
			LPDIRECT3DDEVICE9   pDevice, 
			LPD3DXSPRITE*       ppSprite);

	typedef D3DXMATRIX* (WINAPI *D3DXMatrixMultiply_fun_type)
    ( D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2 );

	typedef LPCSTR (WINAPI *D3DXGetPixelShaderProfile_fun_type)(
        LPDIRECT3DDEVICE9               pDevice);

	typedef LPCSTR (WINAPI *D3DXGetVertexShaderProfile_fun_type)(
        LPDIRECT3DDEVICE9               pDevice);

	typedef HRESULT (WINAPI
    *D3DXCompileShader_fun_type)(
        LPCSTR                          pSrcData,
        UINT                            SrcDataLen,
        CONST D3DXMACRO*                pDefines,
        LPD3DXINCLUDE                   pInclude,
        LPCSTR                          pFunctionName,
        LPCSTR                          pProfile,
        DWORD                           Flags,
        LPD3DXBUFFER*                   ppShader,
        LPD3DXBUFFER*                   ppErrorMsgs,
        LPD3DXCONSTANTTABLE*            ppConstantTable);
	
	HMODULE d3dx9_hmodule;
	bool m_bInitDone;
public:
	D3DXCompileShader_fun_type				D3DXCompileShader_fun;
	D3DXGetVertexShaderProfile_fun_type 	D3DXGetVertexShaderProfile_fun;
	D3DXCreateFontA_fun_type				D3DXCreateFontA_fun;
	D3DXCreateSprite_fun_type				D3DXCreateSprite_fun;
	D3DXMatrixMultiply_fun_type				D3DXMatrixMultiply_fun;
	D3DXGetPixelShaderProfile_fun_type		D3DXGetPixelShaderProfile_fun;

	bool Init()
	{
		g_Log->Info("Searching for d3dx9 dll...");
		m_bInitDone = true;
		static const std::string suffix[20] = {"24", "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41", "42","43"};
		for (size_t ii = 20; ii > 0; --ii)
		{
			std::string modname = "d3dx9_" + suffix[ii-1] + ".dll";
			d3dx9_hmodule = LoadLibraryA(modname.c_str());
			if (d3dx9_hmodule == NULL)
			{
				g_Log->Error("Unable to load module %s",modname.c_str());
			}
			else
			{
				g_Log->Info("Using module %s",modname.c_str());
				break;
			}
		}
			if (d3dx9_hmodule == NULL)
			{
				g_Log->Error("Unable to find d3dx9_24.dll-d3dx9_43.dll");
				return false;
			}
			D3DXCreateFontA_fun = (D3DXCreateFontA_fun_type)GetProcAddress(d3dx9_hmodule, "D3DXCreateFontA");
			if (D3DXCreateFontA_fun == NULL)
			{
				g_Log->Error("Unable to load D3DXCreateFontA_fun");
				return false;
			}
			D3DXCreateSprite_fun= (D3DXCreateSprite_fun_type)GetProcAddress(d3dx9_hmodule, "D3DXCreateSprite");
			if (D3DXCreateSprite_fun == NULL)
			{
				g_Log->Error("Unable to load D3DXCreateSprite_fun");
				return false;
			}
			D3DXMatrixMultiply_fun = (D3DXMatrixMultiply_fun_type)GetProcAddress(d3dx9_hmodule, "D3DXMatrixMultiply");
			if (D3DXMatrixMultiply_fun == NULL)
			{
				g_Log->Error("Unable to load D3DXMatrixMultiply_fun");
				return false;
			}
			D3DXGetPixelShaderProfile_fun = (D3DXGetPixelShaderProfile_fun_type)GetProcAddress(d3dx9_hmodule, "D3DXGetPixelShaderProfile");
			if (D3DXGetPixelShaderProfile_fun == NULL)
			{
				g_Log->Error("Unable to load D3DXGetPixelShaderProfile_fun");
				return false;
			}
			D3DXGetVertexShaderProfile_fun = (D3DXGetVertexShaderProfile_fun_type)GetProcAddress(d3dx9_hmodule, "D3DXGetVertexShaderProfile");
			if (D3DXGetVertexShaderProfile_fun == NULL)
			{
				g_Log->Error("Unable to load D3DXGetVertexShaderProfile_fun");
				return false;
			}
			D3DXCompileShader_fun = (D3DXCompileShader_fun_type)GetProcAddress(d3dx9_hmodule, "D3DXCompileShader");
			if (D3DXCompileShader_fun == NULL)
			{
				g_Log->Error("Unable to load D3DXCompileShader_fun");
				return false;
			}
		return true;
	}

	const char *Description()
	{
		return "DXDLLFun";
	}
	const bool Shutdown(void)
	{
		if (d3dx9_hmodule != NULL)
			FreeLibrary(d3dx9_hmodule);
		return true;
	}

	static DXDLLFun *Instance()
	{
		static	DXDLLFun	dllfun;

		if( dllfun.SingletonActive() == false )
			printf( "Trying to access shutdown singleton dllfun");

		return( &dllfun );
	}
};

};

#define	g_DLLFun	::Base::DXDLLFun::Instance()