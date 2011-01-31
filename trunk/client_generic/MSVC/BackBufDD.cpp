#ifdef _MSC_VER
// Copyleft 2005 Chris Korda
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or any later version.
/*
        chris korda
 
		revision history:
		rev		date	comments
        00      10may05	initial version
        01      11may05	add error strings
        02      12may05	adapt for DX7
        03      20may05	make DX7 optional
        04      05jun05	add mirror blit
        05      22jun05	add SetOptions
        06      29jun05	add mirror precision option
        07      10jul05	add auto memory option
        08      14oct05	add exclusive mode
        09      14oct05	kill DirectDraw's leaky timer
		10		19oct05	in SetExclusive, save/restore window placement
		11		26oct05	don't set position before restoring placement
		12		05may06	add GetErrorString

        DirectDraw back buffer for off-screen drawing
 
*/
#include <algorithm>
#include "BackBufDD.h"
#include "log.h"

#define DIRDRAWERR(x) {x, #x},
const CBackBufDD::ERRTRAN CBackBufDD::m_ErrTran[] = {
#include "DirDrawErrs.h"
{0, NULL}};

CBackBufDD::CBackBufDD()
{
	m_dd = NULL;
	m_Front = NULL;
	m_Back = NULL;
	m_Mirror = NULL;
	m_DrawBuf = NULL;
	m_Clipper = NULL;
	m_Main = NULL;
	m_View = NULL;
	m_hr = 0;
	m_Width = 0;
	m_Height = 0;
	m_IsMirrored = FALSE;
	m_IsExclusive = FALSE;
	m_ContextsUsed = true;
	m_Options = OPT_AUTO_MEMORY | OPT_MIRROR_PRECISE;
	ZeroMemory(&m_PreExcl, sizeof(m_PreExcl));
}

CBackBufDD::~CBackBufDD()
{
	Destroy();
}

BOOL CBackBufDD::Create(HWND Main, HWND View, GUID *Driver, bool Exclusive)
{
	//OutputDebugStringA("CBackBufDD::Create");
	Destroy();

	if (FAILED(m_hr = DirectDrawCreateEx(Driver, (VOID **)&m_dd, IID_IDirectDraw7, NULL)))
	{
		//OutputDebugStringA("CBackBufDD::Create DirectDrawCreateEx");
		return(FALSE);
	}

	m_IsExclusive = Exclusive;	// order matters
	int	mode = Exclusive ? (DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN) : DDSCL_NORMAL;
	if (FAILED(m_hr = m_dd->SetCooperativeLevel(Exclusive ? Main : NULL, mode)))
	{
		//OutputDebugStringA("SetCooperativeLevel");
		return(FALSE);
	}
	// In exclusive mode, DirectDraw creates a 1.5 second periodic timer for
	// unknown reasons.  If the app is switched to exclusive mode and back in
	// less than 1.5 seconds, DirectDraw fails to clean up the timer, and the
	// app's main window receives spurious timer messages for timer ID 16962.
	// KillTimer(Main, 16962);	// nuke it, seems to work fine
	m_Main = Main;
	m_View = View;
	return(TRUE);
}

void CBackBufDD::Destroy()
{
	DeleteSurface();
	if (!IsCreated())
		return;
	m_dd->Release();
	m_dd = NULL;
}

bool CBackBufDD::CreateSurface(int Width, int Height)
{
	//OutputDebugStringA("CBackBufDD::CreateSurface"); 
	//g_Log->Info("m_dd.create width %u height %u",Width,Height);
	if (!IsCreated())
	{
		//OutputDebugStringA("m_dd == NULL");
		return(FALSE);
	}
	Width = (std::max)(Width, 1);	// CreateSurface won't accept zero
	Height = (std::max)(Height, 1);
	DeleteSurface();
	DDSURFACEDESC2	sd = {0};
	sd.dwSize = sizeof(sd);
	if (m_IsExclusive) {
		sd.dwFlags 	= DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		sd.dwBackBufferCount = 2;	// prevents tearing
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Front, NULL)))
			return(FALSE);
		DDSCAPS2		caps = {0};
		caps.dwCaps = DDSCAPS_BACKBUFFER;
		if (FAILED(m_hr = m_Front->GetAttachedSurface(&caps, &m_Back)))
			return(FALSE);
		sd.dwFlags 	= DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE
			| DDSCAPS_SYSTEMMEMORY;	// draw to system memory in mirror mode
		sd.dwWidth	= Width >> 1;	// only need upper-left quadrant
		sd.dwHeight	= Height >> 1;
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Mirror, NULL)))
			return(FALSE);
		m_DrawBuf = m_IsMirrored ? m_Mirror : m_Back;
	} else {
		sd.dwFlags = DDSD_CAPS;
		sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Front, NULL)))
		{
			//OutputDebugStringA("m_dd->CreateSurface(&sd, &m_Front, NULL) 1");
			return(FALSE);
		}
		sd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		// NOTE: must specify DDSCAPS_3DDEVICE to use Direct3D hardware
		sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
		// NOTE: mirroring may not work if DDSCAPS_VIDEOMEMORY is specified
		bool	UseVideoMem;
		if (m_Options & OPT_AUTO_MEMORY)
			UseVideoMem = !m_IsMirrored;
		else
			UseVideoMem = (m_Options & OPT_USE_VIDEO_MEM) != 0;
		if (UseVideoMem)
			sd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		else
			sd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		if (m_IsMirrored) {
			sd.dwWidth		= (Width + 1) >> 1;	// only need upper-left quadrant
			sd.dwHeight		= (Height + 1) >> 1;
			if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Back, NULL)))
			{
				//OutputDebugStringA("m_dd->CreateSurface(&sd, &m_Back, NULL) 2");
				return(FALSE);
			}
			if (m_Options & OPT_MIRROR_PRECISE) {	// mirror to intermediate surface
				sd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE
					| DDSCAPS_VIDEOMEMORY;
				sd.dwWidth	= Width;
				sd.dwHeight	= Height;
				if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Mirror, NULL)))
				{
					//OutputDebugStringA("m_dd->CreateSurface(&sd, &m_Mirror, NULL) 3");
					return(FALSE);
				}
			}
		} else {
			sd.dwWidth = Width;
			sd.dwHeight	= Height;
			if (FAILED(m_hr = m_dd->CreateSurface(&sd, &m_Back, NULL)))
			{
				//OutputDebugStringA("m_dd->CreateSurface(&sd, &m_Back, NULL) 4");
				return(FALSE);
			}
		}
		
		m_DrawBuf = m_Back;
		
		if (FAILED(m_hr = m_dd->CreateClipper(0, &m_Clipper, NULL)))
		{
			//OutputDebugStringA("CBackBufDD::SetClipper1"); 
			return(FALSE);
		}
		if (FAILED(m_hr = m_Clipper->SetHWnd(0, m_View)))
		{
			//OutputDebugStringA("CBackBufDD::SetClipper2"); 
			return(FALSE);
		}
		//RECT wrect;
		////GetWindowRect(m_View, &wrect);
		//GetClientRect(m_View, &wrect);
		//ClientToScreen(m_View, (LPPOINT)&wrect.left);
		//ClientToScreen(m_View, (LPPOINT)&wrect.right);
		//LPRGNDATA rgn = new RGNDATA[sizeof(RGNDATAHEADER)+sizeof(RECT)];
		//RECT *rect = (RECT*)rgn->Buffer ;
		//rect->bottom = wrect.bottom;
		//rect->left = wrect.left;
		//rect->right = wrect.right;
		//rect->top = wrect.top;
		//rgn->rdh.dwSize = sizeof (RGNDATAHEADER);
		//rgn->rdh.iType = RDH_RECTANGLES;
		//rgn->rdh.nCount = 1;
		//rgn->rdh.nRgnSize = sizeof(RECT);
		//m_Clipper->SetClipList(rgn, 0);
		if (FAILED(m_hr = m_Front->SetClipper(m_Clipper)))
		{
			//OutputDebugStringA("CBackBufDD::SetClipper3"); 
			return(FALSE);
		}
		//m_Clipper->Release();
	}
	m_Width = Width;
	m_Height = Height;
	return(TRUE);
}

void CBackBufDD::DeleteSurface()
{
	//OutputDebugStringA("delete surface");
	if (m_Front != NULL)
	{
		//OutputDebugStringA("m_Front != NULL");
		if (m_Back != NULL) {
			m_Back->Release();
			m_Back = NULL;
		}
		if (m_Mirror != NULL) {
			m_Mirror->Release();
			m_Mirror = NULL;
		}

		m_Front->Release();
		m_Front = NULL;

		if (m_Clipper != NULL) {
			//OutputDebugStringA("delete surface - releasing clipper");
			m_Clipper->Release();
			m_Clipper = NULL;
		}

	}
	m_DrawBuf = NULL;
}

bool CBackBufDD::Blt()
{
	if (!IsCreated())
		return false;
	//OutputDebugStringA("CBackBufDD::Blt()");
	enum {
		WAIT	= DDBLT_WAIT,
		WAITFX	= DDBLT_WAIT | DDBLT_DDFX
	};
	while (1) {
		HRESULT hRet;
		if (m_IsExclusive) {
			if (m_IsMirrored) {	// mirrored blit
				int	w = m_Width >> 1;
				int	h = m_Height >> 1;
				RECT	r = {0, 0, w, h};
				DDBLTFX	fx = {0};
				fx.dwSize = sizeof(fx);
				hRet = m_Back->Blt(&r, m_Mirror, NULL, WAIT, &fx);	// upper left
				if (hRet == DD_OK) {
					fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
					r.left += w;
					r.right += w;
					hRet = m_Back->Blt(&r, m_Mirror, NULL, WAITFX, &fx);	// upper right
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
						r.bottom +=h;
						r.top +=h;
						hRet = m_Back->Blt(&r, m_Mirror, NULL, WAITFX, &fx);	// lower right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
							r.left -= w;
							r.right -= w;
							hRet = m_Back->Blt(&r, m_Mirror, NULL, WAITFX, &fx);	// lower left
							if (hRet == DD_OK)
								hRet = m_Front->Flip(NULL, DDFLIP_WAIT);	// update display
						}
					}
				}
			} else
				hRet = m_Front->Flip(NULL, DDFLIP_WAIT);	// update display
		} else {
			POINT	org = {0, 0};
			ClientToScreen(m_View, &org);
			if (m_IsMirrored) {	// mirrored blit
				RECT	r = {0, 0, (m_Width + 1) >> 1, (m_Height + 1) >> 1};
				int	w = (m_Width >> 1);
				int	h = (m_Height >> 1);
				DDBLTFX	fx = {0};
				fx.dwSize = sizeof(fx);
				if (m_Mirror != NULL) {	// mirror to intermediate surface
					hRet = m_Mirror->Blt(&r, m_Back, NULL, WAIT, NULL);	// upper left
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
						r.left += w;
						r.right += w;
						hRet = m_Mirror->Blt(&r, m_Back, NULL, WAITFX, &fx);	// upper right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
							r.top += h;
							r.bottom += h;
							hRet = m_Mirror->Blt(&r, m_Back, NULL, WAITFX, &fx);	// lower right
							if (hRet == DD_OK) {
								fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
								r.left -= w;
								r.right -= w;
								hRet = m_Mirror->Blt(&r, m_Back, NULL, WAITFX, &fx);	// lower left
								if (hRet == DD_OK) {	// blit mirrored image to display
									RECT	wr = {0, 0, m_Width, m_Height};
									wr.left += org.x;
									wr.right += org.x;
									wr.top += org.y;
									wr.bottom += org.y;
									hRet = m_Front->Blt(&wr, m_Mirror, NULL, WAIT, NULL);
								}
							}
						}
					}
				} else {	// mirror directly to display; quick and dirty
					r.left += org.x;
					r.right += org.x;
					r.top += org.y;
					r.bottom += org.y;
					hRet = m_Front->Blt(&r, m_Back, NULL, WAIT, NULL);	// upper left
					if (hRet == DD_OK) {
						fx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;
						r.left += w;
						r.right += w;
						hRet = m_Front->Blt(&r, m_Back, NULL, WAITFX, &fx);	// upper right
						if (hRet == DD_OK) {
							fx.dwDDFX = DDBLTFX_MIRRORUPDOWN | DDBLTFX_MIRRORLEFTRIGHT;
							r.top += h;
							r.bottom += h;
							hRet = m_Front->Blt(&r, m_Back, NULL, WAITFX, &fx);	// lower right
							if (hRet == DD_OK) {
								fx.dwDDFX = DDBLTFX_MIRRORUPDOWN;
								r.left -= w;
								r.right -= w;
								hRet = m_Front->Blt(&r, m_Back, NULL, WAITFX, &fx);	// lower left
							}
						}
					}
				}
			} else {	// ordinary blit
				RECT	r = {0, 0, m_Width, m_Height};
				r.left += org.x;
				r.right += org.x;
				r.top += org.y;
				r.bottom += org.y;
				hRet = m_Front->Blt(&r, m_Back, NULL, WAIT, NULL);
			}
		}
		if (hRet == DD_OK)
			break;
		else if (hRet == DDERR_SURFACELOST) {
			if (FAILED(m_Front->Restore()) || FAILED(m_Back->Restore()))
				return(FALSE);
		} else if (hRet != DDERR_WASSTILLDRAWING)
			return(FALSE);
	}
	return(TRUE);
}

LPCSTR CBackBufDD::GetErrorString(HRESULT hr)
{
	for (int i = 0; m_ErrTran[i].Text != NULL; i++) {
		if (m_ErrTran[i].Code == hr)
			return(m_ErrTran[i].Text);
	}
	return("unknown error");
}

void CBackBufDD::SetMirror(bool Enable)
{
	if (m_IsExclusive)
		m_DrawBuf = Enable ? m_Mirror : m_Back;
	m_IsMirrored = Enable;
}

void CBackBufDD::SetOptions(int Options)
{
	m_Options = Options;
}

bool CBackBufDD::SetExclusive(HWND Main, HWND View, bool Enable)
{
	GUID	MonGuid, *Device = NULL;
	RECT	rc;
	if (Enable) {	// if exclusive mode
		GetWindowPlacement(Main, &m_PreExcl);	// save window placement
		// get rect and handle of monitor that this window is mostly on
		HMONITOR	hMon = GetFullScreenRect(View, rc);
		if (GetMonitorGUID(hMon, MonGuid))	// if monitor's GUID is available
			Device = &MonGuid;	// pass GUID to Create for hardware acceleration
		m_Width = rc.right - rc.left;
		m_Height = rc.bottom - rc.top;
	}
	if (!Create(Main, View, Device, Enable))
		return(FALSE);
	if (!CreateSurface(m_Width, m_Height))
		return(FALSE);
	if (Enable) {
		SetWindowPos(Main, HWND_TOPMOST,	// set topmost attribute
			rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0);	// and go full-screen
	} else {
		SetWindowPos(Main, HWND_NOTOPMOST,	// clear topmost attribute
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);	// but don't set position
		SetWindowPlacement(Main, &m_PreExcl);	// restore previous placement
	}
	return(TRUE);
}

HMONITOR CBackBufDD::GetFullScreenRect(HWND hWnd, RECT& rc)
{
	RECT	wr;
	::GetWindowRect(hWnd, &wr);
	// try to get screen size from monitor API in case we're dual-monitor
	MONITORINFO	mi;
	mi.cbSize = sizeof(mi);
	HMONITOR	hMon = MonitorFromRect(&wr, MONITOR_DEFAULTTONEAREST);
	if (hMon != NULL && GetMonitorInfo(hMon, &mi)) {
		rc = mi.rcMonitor;
	} else {	// fall back to older API
		rc.bottom = GetSystemMetrics(SM_CYSCREEN);
		rc.left = 0;
		rc.right = GetSystemMetrics(SM_CXSCREEN);
		rc.top	= 0;
	}
	return(hMon);
}

BOOL WINAPI CBackBufDD::DDEnumCallbackEx(GUID FAR *lpGUID, LPTSTR lpDriverDescription, 
	LPTSTR lpDriverName, LPVOID lpContext, HMONITOR hm)
{
	MONGUID	*mg = (MONGUID *)lpContext;
	if (hm == mg->Mon) {	// if it's the monitor we're looking for
		mg->Guid = *lpGUID;	// pass its GUID to caller
		mg->Valid = TRUE;	// tell caller we found it
		return(DDENUMRET_CANCEL);	// stop enumerating
	}
	return(DDENUMRET_OK);	// continue enumerating
}
 
bool CBackBufDD::GetMonitorGUID(HMONITOR hMon, GUID& MonGuid)
{
	// DirectDrawEnumerateEx has to be manually imported from the DirectDraw DLL
	HINSTANCE h = LoadLibrary(L"ddraw.dll");
	if (!h)
		return(FALSE);
	LPDIRECTDRAWENUMERATEEX	lpDDEnumEx;
	lpDDEnumEx = (LPDIRECTDRAWENUMERATEEXW)GetProcAddress(h, "DirectDrawEnumerateExW");
	bool	retc = FALSE;
	if (lpDDEnumEx) {	// if the function was imported
		MONGUID	mg = {0};
		mg.Mon = hMon;	// tell callback which monitor to look for
		lpDDEnumEx(DDEnumCallbackEx, &mg, DDENUM_ATTACHEDSECONDARYDEVICES);
		if (mg.Valid) {	// if callback found the monitor
			MonGuid = mg.Guid;	// pass its GUID to caller
			retc = TRUE;
		}
	}
    FreeLibrary(h);
	return(retc);
}

void CBackBufDD::DrawQuad(const Base::Math::CRect &_rect, const Base::Math::CVector4 &_color, const Base::Math::CRect &_uvrect )
{
	if (!IsCreated())
		return;
	//OutputDebugStringA("CBackBufDD::DrawQuad()");
	if (m_ContextsUsed == false && m_DrawBuf != NULL)
	{
		HDC hdc;
		if  (m_DrawBuf->GetDC(&hdc) == DD_OK)
		{
			while (m_Tex.size() > 0)
			{
				LPDIRECTDRAWSURFACE7 curtex = m_Tex.front();
				m_Tex.pop();
				int curwidth = m_SrcWidth.front();
				m_SrcWidth.pop();
				int curheight = m_SrcHeight.front();
				m_SrcHeight.pop();
				//RECT coords;
				//coords.left = int(_rect.m_X0 * m_Width);
				//coords.top = int(_rect.m_Y0 * m_Height);
				//coords.right = int(_rect.m_X1 * m_Width);
				//coords.bottom = int(_rect.m_Y1  * m_Height);
				//m_DrawBuf->Blt(&coords, curtex, NULL, 0, NULL);
				HDC sdc;
				if (curtex->GetDC(&sdc) == DD_OK)
				{
					SetStretchBltMode(hdc, COLORONCOLOR);
					StretchBlt(hdc,
						int(_rect.m_X0 * m_Width),
						int(_rect.m_Y0 * m_Height),
						int((_rect.m_X1 - _rect.m_X0) * m_Width),
						int((_rect.m_Y1 - _rect.m_Y0) * m_Height),
						sdc, 0, 0, curwidth, curheight, SRCCOPY);
					curtex->ReleaseDC(sdc);
					curtex->Release();
				}
			}
			m_DrawBuf->ReleaseDC(hdc);
		}
		while (m_Tex.size() > 100)
		{
			m_Tex.front()->Release();
			m_Tex.pop();
			m_SrcWidth.pop();
			m_SrcHeight.pop();
		}
		m_ContextsUsed = true;
	}
}


void CBackBufDD::SetContexts(LPDIRECTDRAWSURFACE7 _tex, int _SrcWidth, int _SrcHeight)
{
	if (!IsCreated())
		return;
	//OutputDebugStringA("CBackBufDD::SetContexts()");
	m_ContextsUsed = false;
	m_Tex.push(_tex);
	m_SrcWidth.push(_SrcWidth);
	m_SrcHeight.push(_SrcHeight);
}

HRESULT CBackBufDD::GetDC(HDC FAR *lphDC)
{
	if (m_DrawBuf != NULL)
		return(m_DrawBuf->GetDC(lphDC));
	else
		return DDERR_GENERIC;
}

HRESULT CBackBufDD::ReleaseDC(HDC hDC)
{
	if (m_DrawBuf != NULL)
		return(m_DrawBuf->ReleaseDC(hDC));
	else
		return DDERR_GENERIC;
}

HRESULT CBackBufDD::GetLastError() const
{
	return(m_hr);
}

LPCSTR CBackBufDD::GetLastErrorString() const
{
	return(GetErrorString(GetLastError()));
}

bool CBackBufDD::IsCreated() const
{
	return(m_dd != NULL);
}

bool CBackBufDD::IsMirrored() const
{
	return(m_IsMirrored);
}

int CBackBufDD::GetOptions() const
{
	return(m_Options);
}

SIZE CBackBufDD::GetSize() const
{
	SIZE temp = {m_Width, m_Height};
	return temp;
}

bool CBackBufDD::IsExclusive() const
{
	return(m_IsExclusive);
}

bool CBackBufDD::IsSurface() const
{
	return(m_Back != NULL);
}

HRESULT CBackBufDD::CreateSurface(LPDDSURFACEDESC2 SurfaceDesc, LPDIRECTDRAWSURFACE7 FAR *Surface)
{
	if (m_dd != NULL)
		return(m_dd->CreateSurface(SurfaceDesc, Surface, NULL));
	else
		return DDERR_GENERIC;
}

#endif