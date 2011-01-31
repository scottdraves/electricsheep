#ifndef _DisplayGL_H_

#include	"base.h"
#include	"DisplayOutput.h"

//	Don't include any of these directly, they will typedef themselves to CDisplayGL.
#ifdef	WIN32
	#include	"wgl.h"
#else
#ifdef MAC
	#include	"mgl.h"
#else
	#include	"glx.h"
#endif
#endif

#define _DisplayGL_H_	//	Define here so we can test for include mess.
#endif
