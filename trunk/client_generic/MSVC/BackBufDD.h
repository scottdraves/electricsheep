#pragma once
// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10may05	initial version
		01		05may06	add CreateSurface wrapper

        DirectDraw back buffer for off-screen drawing
 
*/
#include "base.h"
#include "rect.h"
#include "vector4.h"
#include "wobject.h"
#include <ddraw.h>
#include <queue>

class CBackBufDD
{
public:
	enum {
		OPT_AUTO_MEMORY		= 0x01,	// automatically decide back buffer location;
									// otherwise OPT_USE_VIDEO_MEM sets location
		OPT_USE_VIDEO_MEM	= 0x02,	// if specified, create back buffer in video 
									// memory, else create it in system memory;
									// ignored if OPT_AUTO_MEMORY is specified
		OPT_MIRROR_PRECISE	= 0x04	// make mirroring more precise at the expense
									// of mirroring to an intermediate back buffer
	};
	CBackBufDD();
	~CBackBufDD();
	BOOL	Create(HWND Main, HWND View, GUID *Driver = NULL, bool Exclusive = FALSE);
	void	Destroy();
	void	DeleteSurface();
	bool	CreateSurface(int Width, int Height);
	void	DrawQuad(const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const Base::Math::CRect &_uvrect );
	void	SetContexts(LPDIRECTDRAWSURFACE7 _tex, int _SrcWidth, int _SrcHeight);
	HRESULT	GetDC(HDC FAR *lphDC);
	HRESULT	ReleaseDC(HDC hDC);
	bool	Blt();
	HRESULT	GetLastError() const;
	LPCSTR	GetLastErrorString() const;
	bool	IsCreated() const;
	void	SetMirror(bool Enable);
	bool	IsMirrored() const;
	void	SetOptions(int Options);
	int		GetOptions() const;
	bool	SetExclusive(HWND Main, HWND View, bool Enable);
	SIZE	GetSize() const;
	bool	IsExclusive() const;
	bool	IsSurface() const;
	static	HMONITOR	GetFullScreenRect(HWND hWnd, RECT& rc);
	static	bool	GetMonitorGUID(HMONITOR hMon, GUID& MonGuid);
	static	LPCSTR	GetErrorString(HRESULT hr);
	HRESULT	CreateSurface(LPDDSURFACEDESC2 SurfaceDesc, LPDIRECTDRAWSURFACE7 FAR *Surface);

private:
	typedef	struct tagERRTRAN {
		HRESULT	Code;
		LPCSTR	Text;
	} ERRTRAN;
	typedef struct tagMONGUID {
		HMONITOR	Mon;
		GUID		Guid;
		BOOL		Valid;
	} MONGUID;
	static	const	ERRTRAN	m_ErrTran[];	// map DirectDraw error codes to text
	LPDIRECTDRAW7	m_dd;			// pointer to DirectDraw instance
	LPDIRECTDRAWSURFACE7	m_Front;	// pointer to front buffer
	LPDIRECTDRAWSURFACE7	m_Back;		// pointer to back buffer surface
	LPDIRECTDRAWSURFACE7	m_Mirror;	// pointer to mirroring buffer
	LPDIRECTDRAWSURFACE7	m_DrawBuf;	// pointer to buffer we draw to
    LPDIRECTDRAWCLIPPER	m_Clipper;	// pointer to clipper
	HWND	m_Main;				// top-level owner
	HWND	m_View;				// window to clip to
	HRESULT	m_hr;				// most recent DirectDraw result code
	int		m_Width;			// width of window, in client coords
	int		m_Height;			// height of window, in client coords
	bool	m_IsMirrored;		// true if we're mirroring
	bool	m_IsExclusive;		// true if we're in full-screen exclusive mode
	int		m_Options;			// display options; see enum above

	std::queue<LPDIRECTDRAWSURFACE7>	m_Tex;
	std::queue<int>		m_SrcWidth;
	std::queue<int>		m_SrcHeight;
	bool	m_ContextsUsed;
	WINDOWPLACEMENT	m_PreExcl;	// window placement before exclusive mode

	static	BOOL WINAPI DDEnumCallbackEx(GUID FAR *lpGUID, LPTSTR lpDriverDescription,
		LPTSTR lpDriverName, LPVOID lpContext, HMONITOR hm);
};