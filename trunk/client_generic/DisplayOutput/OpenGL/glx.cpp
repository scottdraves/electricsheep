#ifndef	WIN32

#include <string>
#include <iostream>
#include <assert.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xatom.h>

#include "glx.h"
#include "Log.h"
#include "Exception.h"

namespace	DisplayOutput
{

using namespace std;

typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
} MotifWmHints, MwmHints;

#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)

#define NET_WM_STATE_TOGGLE 2

static Atom XA_NET_WM_STATE;
static Atom XA_NET_WM_STATE_ABOVE;
static Atom XA_WIN_LAYER;
static Atom XA_NET_WM_STATE_ADD;
static Atom XA_NET_WM_STATE_MAXIMIZED_VERT;
static Atom XA_NET_WM_STATE_MAXIMIZED_HORZ;
static Atom XA_NET_WM_STATE_FULLSCREEN;

static bool bScreensaverMode = false;

/*
*/
CUnixGL::CUnixGL() : CDisplayOutput()
{
}

CUnixGL::~CUnixGL()
{
#ifdef LINUX_GNU
  if (!bScreensaverMode) {
#endif
    XUnmapWindow (m_pDisplay, m_Window);
    XDestroyWindow (m_pDisplay, m_Window);
#ifdef LINUX_GNU
  }
#endif
  XCloseDisplay (m_pDisplay);

#ifdef LINUX_GNU
    if ( !bScreensaverMode )
      /* disable screensaver and screen blanking in non-screensaver mode */
      int dummy = system( "xset s on 2>/dev/null; xset +dpms 2>/dev/null; gconftool-2 --set --type bool \
                              /apps/gnome-screensaver/idle_activation_enabled true 2>/dev/null" );
#endif

}

#ifdef LINUX_GNU

static Bool WaitForNotify( Display *dpy, XEvent *event, XPointer arg ) {
  return (event->type == MapNotify) && (event->xmap.window == (Window) arg);
}

#endif


bool	CUnixGL::Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen )
{
    m_Width = _width;
    m_Height = _height;

    m_pDisplay = XOpenDisplay(0);
    assert(m_pDisplay);

    m_WidthFS = WidthOfScreen(DefaultScreenOfDisplay(m_pDisplay));
    m_HeightFS = HeightOfScreen(DefaultScreenOfDisplay(m_pDisplay));

    XA_NET_WM_STATE = XInternAtom(m_pDisplay, "_NET_WM_STATE", False);
    XA_NET_WM_STATE_ABOVE = XInternAtom(m_pDisplay, "_NET_WM_ABOVE", False);
    XA_WIN_LAYER = XInternAtom(m_pDisplay, "_WIN_LAYER", False);

    int renderEventBase;
    int renderErrorBase;

    if( !XRenderQueryExtension( m_pDisplay, &renderEventBase, &renderErrorBase ) )
    {
        g_Log->Error( "No RENDER extension found!" );
        return false;
    }

    GLXFBConfig     *pFbConfig = NULL;
    GLXFBConfig     renderFBConfig;
    XVisualInfo     *pVisualInfo = NULL;
    XSetWindowAttributes    winAttributes;
    int numElements;
    XEvent event;

    int singleBufferAttributess[] = {
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE,   GLX_RGBA_BIT,
		GLX_RED_SIZE,      1,			//	Request a single buffered color buffer
		GLX_GREEN_SIZE,    1,			//	with the maximum number of color bits
		GLX_BLUE_SIZE,     1,			//	for each component.
		None
    };

	int doubleBufferAttributes[] = {
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE,   GLX_RGBA_BIT,
		GLX_DOUBLEBUFFER,  True,		//	Request a double-buffered color buffer with
		GLX_RED_SIZE,      1,			//	the maximum number of bits per component.
		GLX_GREEN_SIZE,    1,
		GLX_BLUE_SIZE,     1,
		None
    };

	//	First check doublebuffered...
	pFbConfig = glXChooseFBConfig( m_pDisplay, DefaultScreen( m_pDisplay ), doubleBufferAttributes, &numElements );
	if( pFbConfig )
		g_Log->Info( "Double buffered..." );

	//	...then singlebuffered.
    if( pFbConfig == NULL )
    {
		pFbConfig = glXChooseFBConfig( m_pDisplay, DefaultScreen( m_pDisplay ), singleBufferAttributess, &numElements );
		if( pFbConfig )
			g_Log->Info( "Single buffered..." );
    }


	//	Final fail.
    assert( pFbConfig );

    pVisualInfo = glXGetVisualFromFBConfig( m_pDisplay, pFbConfig[0] );
    renderFBConfig = pFbConfig[0];

    winAttributes.colormap = XCreateColormap( m_pDisplay, RootWindow(m_pDisplay, pVisualInfo->screen),pVisualInfo->visual, AllocNone );

    winAttributes.border_pixel = 0;
    winAttributes.event_mask = StructureNotifyMask | ButtonPressMask | KeyPressMask;
    winAttributes.override_redirect = true;

#ifdef LINUX_GNU
     const char *xss_id = getenv("XSCREENSAVER_WINDOW");
     if (xss_id && *xss_id) {
       int numReturned;
       XVisualInfo *xvis;
       XVisualInfo xvtmpl;
       XWindowAttributes attr;

       unsigned long id = 0;
       sscanf (xss_id, " 0x%lx", &id);
       m_Window = (Window) id;
       m_GlxWindow = m_Window;
       XGetWindowAttributes(m_pDisplay, m_Window, &attr);
       
       xvtmpl.visual=attr.visual;
       xvtmpl.visualid=XVisualIDFromVisual(attr.visual);
       
       xvis=XGetVisualInfo(m_pDisplay, VisualIDMask, &xvtmpl, &numReturned);
       
       assert (numReturned>0);
       
       m_GlxContext = glXCreateContext ( m_pDisplay, &xvis[0], 0, GL_TRUE);
       
       glXMakeCurrent ( m_pDisplay, m_Window, m_GlxContext);


       m_Width=attr.width;
       m_Height=attr.height;

       bScreensaverMode = true;
     }
     else {
       m_Window = XCreateWindow( m_pDisplay, RootWindow( m_pDisplay, pVisualInfo->screen ), 0, 0, 
				 _bFullscreen?m_WidthFS:m_Width, _bFullscreen?m_HeightFS:m_Height, 0, 
				 pVisualInfo->depth, InputOutput,
				 pVisualInfo->visual, CWBorderPixel | CWColormap | CWEventMask, &winAttributes);

       setFullScreen( _bFullscreen );

       XMapRaised( m_pDisplay, m_Window );
       XIfEvent( m_pDisplay, &event, WaitForNotify, (XPointer) m_Window );

       // need to call this twice !!
       setFullScreen( _bFullscreen );

       m_GlxContext = glXCreateNewContext(m_pDisplay, renderFBConfig, GLX_RGBA_TYPE, 0, GL_TRUE);
       m_GlxWindow = glXCreateWindow(m_pDisplay, renderFBConfig, m_Window, 0);
       glXMakeContextCurrent(m_pDisplay, m_GlxWindow, m_GlxWindow, m_GlxContext);

       /* disable screensaver and screen blanking in non-screensaver mode */
       int dummy = system( "xset s off 2>/dev/null; xset -dpms 2>/dev/null; gconftool-2 --set --type bool \
                                /apps/gnome-screensaver/idle_activation_enabled false 2>/dev/null" );
     }

     Cursor invisibleCursor;
     Pixmap bitmapNoData;
     XColor black;
     static char noData[] = { 0,0,0,0,0,0,0,0 };
     black.red = black.green = black.blue = 0;

     bitmapNoData = XCreateBitmapFromData( m_pDisplay, m_Window, noData, 8, 8 );
     invisibleCursor = XCreatePixmapCursor( m_pDisplay, bitmapNoData, bitmapNoData, 
					    &black, &black, 0, 0 );
     XDefineCursor( m_pDisplay, m_Window, invisibleCursor );
     XFreeCursor( m_pDisplay, invisibleCursor );

#else
    m_Window = XCreateWindow( m_pDisplay, RootWindow( m_pDisplay, pVisualInfo->screen ), 0, 0, 
			      _bFullscreen?m_WidthFS:m_Width, _bFullscreen?m_HeightFS:m_Height, 0, 
			      pVisualInfo->depth, InputOutput,
			      pVisualInfo->visual, CWBorderPixel | CWColormap | CWEventMask, &winAttributes);

    if (!bScreensaverMode) setFullScreen( _bFullscreen );
    XMapRaised( m_pDisplay, m_Window );
    if (!bScreensaverMode && _bFullscreen) XIfEvent( m_pDisplay, &event, WaitForNotify, (XPointer) m_Window );

    m_GlxContext = glXCreateNewContext(m_pDisplay, renderFBConfig, GLX_RGBA_TYPE, 0, GL_TRUE);
    m_GlxWindow = glXCreateWindow(m_pDisplay, renderFBConfig, m_Window, 0);
    XMapWindow (m_pDisplay, m_Window);
    glXMakeContextCurrent(m_pDisplay, m_GlxWindow, m_GlxWindow, m_GlxContext);
#endif

    Atom wmDelete = XInternAtom(m_pDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(m_pDisplay, m_Window, &wmDelete, 1);

    toggleVSync();

    XFree (pVisualInfo);

	int error = glGetError();
	if( error != GL_NO_ERROR )
	{
		std::string msg = "";

		if( error == GL_INVALID_ENUM )	msg = "GL_INVALID_ENUM";
		else if( error == GL_INVALID_VALUE ) msg = "GL_INVALID_VALUE";
		else if( error ==    GL_INVALID_OPERATION) msg = "GL_INVALID_OPERATION";
		else if( error ==    GL_STACK_OVERFLOW)	msg = "GL_STACK_OVERFLOW";
		else if( error ==    GL_STACK_UNDERFLOW)	msg = "GL_STACK_UNDERFLOW";
		else if( error ==    GL_OUT_OF_MEMORY)	msg = "GL_OUT_OF_MEMORY";
		else if( error ==    GL_INVALID_FRAMEBUFFER_OPERATION_EXT)	msg = "GL_INVALID_FRAMEBUFFER_OPERATION_EXT";
		else msg = "Unrecognized OpenGL error";

		g_Log->Error( "%s in %s(%d)", msg.c_str(), __FILE__, __LINE__ );
		return false;
	}

	return true;
}

/*
*/
void CUnixGL::Title( const std::string &_title )
{
    XTextProperty textProp;
    textProp.value = (unsigned char *)_title.c_str();
    textProp.encoding = XA_STRING;
    textProp.format = 8;
    textProp.nitems = _title.length();

    XSetWMName( m_pDisplay, m_Window, &textProp );
}

/*
*/
void CUnixGL::setWindowDecorations( bool enabled )
{
    unsigned char* pucData;
    int iFormat;
    unsigned long ulItems;
    unsigned long ulBytesAfter;
    Atom typeAtom;
    MotifWmHints newHints;
    bool set=false;

    Atom hintsAtom = XInternAtom (m_pDisplay, "_MOTIF_WM_HINTS", True);
    if (hintsAtom != None) {

      XGetWindowProperty (m_pDisplay, m_Window, hintsAtom, 0,
			  sizeof (MotifWmHints) / sizeof (long),
			  False, AnyPropertyType, &typeAtom,
			  &iFormat, &ulItems, &ulBytesAfter, &pucData);

      newHints.flags = MWM_HINTS_DECORATIONS;
      newHints.decorations = enabled ? 1:0;

      XChangeProperty (m_pDisplay, m_Window, hintsAtom, hintsAtom,
		       32, PropModeReplace, (unsigned char *) &newHints,
		       sizeof (MotifWmHints) / sizeof (long));
      set = true;

    }


      /* Now try to set KWM hints */
    hintsAtom = XInternAtom(m_pDisplay, "KWM_WIN_DECORATION", True);
    if (hintsAtom != None) {
      long KWMHints = 0;

      XChangeProperty(m_pDisplay, m_Window, hintsAtom, hintsAtom, 32,
		      PropModeReplace,
		      (unsigned char *) &KWMHints,
		      sizeof(KWMHints) / 4);
      set = true;
    }
    /* Now try to set GNOME hints */
    hintsAtom = XInternAtom(m_pDisplay, "_WIN_HINTS", True);
    if (hintsAtom != None) {
      long GNOMEHints = 0;

      XChangeProperty(m_pDisplay, m_Window, hintsAtom, hintsAtom, 32,
		      PropModeReplace,
		      (unsigned char *) &GNOMEHints,
		      sizeof(GNOMEHints) / 4);
      set = true;
    }
    /* Finally set the transient hints if necessary */
    if (!set) {
      XSetTransientForHint(m_pDisplay, m_Window, RootWindow(m_pDisplay, DefaultScreen(m_pDisplay)));
    }

}


static bool
isWindowMapped(Display *dpy, Window *xWin)
{
  XWindowAttributes attr;

  XGetWindowAttributes(dpy, *xWin, &attr);
  if (attr.map_state != IsUnmapped) {
    return true;
  } else {
    return false;
  }
}





void CUnixGL::setFullScreen(bool enabled)
{
    XWindowChanges changes;
    unsigned int valueMask = CWX | CWY | CWWidth | CWHeight;

    m_FullScreen = enabled;
    setWindowDecorations(!m_FullScreen);

    if (m_FullScreen)
    {

        XA_NET_WM_STATE = XInternAtom(m_pDisplay, "_NET_WM_STATE", False);
	XA_NET_WM_STATE_ADD = XInternAtom(m_pDisplay, "_NET_WM_STATE_ADD", False);

	XA_NET_WM_STATE_MAXIMIZED_VERT = XInternAtom(m_pDisplay, "_NET_WM_STATE_MAXIMIZED_VERT",False);
	XA_NET_WM_STATE_MAXIMIZED_HORZ = XInternAtom(m_pDisplay,"_NET_WM_STATE_MAXIMIZED_HORZ",False);
	XA_NET_WM_STATE_FULLSCREEN = XInternAtom(m_pDisplay,"_NET_WM_STATE_FULLSCREEN",False);
	
	
	if (isWindowMapped(m_pDisplay, &m_Window)) {
	  XEvent e;
	  
	  memset(&e,0,sizeof(e));
	  e.xany.type = ClientMessage; 
	  e.xclient.message_type = XA_NET_WM_STATE;
	  e.xclient.format = 32;
	  e.xclient.window = m_Window;
	  e.xclient.data.l[0] = XA_NET_WM_STATE_ADD;
	  e.xclient.data.l[1] = XA_NET_WM_STATE_FULLSCREEN;
	  e.xclient.data.l[3] = 0l;
	  
	  XSendEvent(m_pDisplay, RootWindow(m_pDisplay, 0), 0,
		     SubstructureNotifyMask | SubstructureRedirectMask, &e);
	} else {
	  int count = 0;
	  Atom atoms[3];
	  
	  atoms[count++] = XA_NET_WM_STATE_FULLSCREEN;
	  atoms[count++] = XA_NET_WM_STATE_MAXIMIZED_VERT;
	  atoms[count++] = XA_NET_WM_STATE_MAXIMIZED_HORZ;
	  XChangeProperty(m_pDisplay, m_Window, XA_NET_WM_STATE, XA_ATOM, 32,
			  PropModeReplace, (unsigned char *)atoms, count);
	}

        changes.x = 0;
        changes.y = 0;
        changes.width = m_WidthFS;
        changes.height = m_HeightFS;
        changes.stack_mode = Above;
        valueMask |= CWStackMode;

	//m_Width = changes.width;
	//m_Height = changes.height;
    }
    else
    {
        changes.x = m_XPosition;
        changes.y = m_YPosition;
        changes.width = m_Width;
        changes.height = m_Height;
    }

    //XSetWindowAttributes winAttrib;
    //winAttrib.override_redirect = True;
    //XChangeWindowAttributes(m_pDisplay, m_Window, CWOverrideRedirect, &winAttrib);
    XMapRaised(m_pDisplay, m_Window);
    //XRaiseWindow(m_pDisplay, m_Window);
    XConfigureWindow(m_pDisplay, m_Window, valueMask, &changes);
    alwaysOnTop();
    //XFlush(m_pDisplay);
}

/*
*/
void CUnixGL::alwaysOnTop()
{
    //~ XClientMessageEvent xev;
    //~ memset(&xev, 0, sizeof(xev));

    //~ xev.type = ClientMessage;
    //~ xev.message_type = XA_NET_WM_STATE;
    //~ xev.display = m_pDisplay;
    //~ xev.window = m_Window;
    //~ xev.format = 32;
    //~ xev.data.l[0] = 10;

    //if (vo_fs_type & vo_wm_STAYS_ON_TOP)
        //xev.data.l[1] = XA_NET_WM_STATE_STAYS_ON_TOP;
    //else if (vo_fs_type & vo_wm_ABOVE)
        //xev.data.l[1] = XA_NET_WM_STATE_ABOVE;
    //else if (vo_fs_type & vo_wm_FULLSCREEN)
    //    xev.data.l[1] = XA_NET_WM_STATE_FULLSCREEN;
    //else if (vo_fs_type & vo_wm_BELOW)
        // This is not fallback. We can safely assume that situation where
        // only NETWM_STATE_BELOW is supported and others not, doesn't exist.
    //    xev.data.l[1] = XA_NET_WM_STATE_BELOW;

    //XSendEvent(m_pDisplay, m_Window, False, SubstructureRedirectMask, reinterpret_cast<XEvent*>(&xev));
    //state = XGetAtomName(mDisplay, xev.data.l[1]);
    //mp_msg(MSGT_VO, MSGL_V,
    //       "[x11] NET style stay on top (layer %d). Using state %s.\n",
    //       layer, state);
    //XFree(state);

    long propvalue = 12;
    XChangeProperty( m_pDisplay, m_Window, XA_WIN_LAYER, XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&propvalue, 1 );
    XRaiseWindow( m_pDisplay, m_Window);
}

/*
*/
void CUnixGL::toggleVSync()
{
    m_VSync = !m_VSync;

    if( GLEE_GLX_SGI_swap_control )
    {
        if( m_VSync )
            glXSwapIntervalSGI(1);
        else
            glXSwapIntervalSGI(2);
    }
}

/*
*/
void CUnixGL::Update()
{
    checkClientMessages();
}

/*
*/
void CUnixGL::SwapBuffers()
{
    glXSwapBuffers( m_pDisplay, m_GlxWindow );
}

/*
*/
/*bool CUnixGL::checkResizeEvent( ResizeEvent &event )
{
    XEvent xEvent;
    bool gotEvent = false;

    //  remove old resize events to prevent delay.
    while( XCheckTypedEvent( m_pDisplay, ConfigureNotify, &xEvent ) )
        gotEvent = true;

    if( gotEvent )
    {
        if( !m_FullScreen )
        {
            m_Width = xEvent.xconfigure.width;
            m_Height = xEvent.xconfigure.height;
            m_XPosition = xEvent.xconfigure.x;
            m_YPosition = xEvent.xconfigure.y;
        }

        event.width = xEvent.xconfigure.width;
        event.height = xEvent.xconfigure.height;
    }

    return gotEvent;
}*/

/*
*/
void CUnixGL::checkClientMessages()
{
    XEvent xEvent;

    if( XCheckTypedEvent( m_pDisplay, ClientMessage, &xEvent ) )
        if( *XGetAtomName(m_pDisplay, xEvent.xclient.message_type) == *"WM_PROTOCOLS" )
            m_bClosed = true;

	//	Keyboard.
    if( XCheckWindowEvent( m_pDisplay, m_Window, KeyPressMask | KeyReleaseMask, &xEvent ) )
    {
		CKeyEvent *spEvent = new CKeyEvent();

        if( xEvent.type == KeyPress )			{	spEvent->m_bPressed = true;	}
        else if( xEvent.type == KeyRelease )	{	spEvent->m_bPressed = false;	}

        KeySym keySymbol = XKeycodeToKeysym( m_pDisplay, xEvent.xkey.keycode, 0 );

        switch( keySymbol )
        {
			case XK_F1:     spEvent->m_Code = CKeyEvent::KEY_F1;	break;
			case XK_F2:     spEvent->m_Code = CKeyEvent::KEY_F2;	break;
			case XK_F3:     spEvent->m_Code = CKeyEvent::KEY_F3;	break;
			case XK_F4:     spEvent->m_Code = CKeyEvent::KEY_F4;	break;
			case XK_F8:     spEvent->m_Code = CKeyEvent::KEY_F8;	break;
			case XK_f:      spEvent->m_Code = CKeyEvent::KEY_F;	break;
			case XK_s:      spEvent->m_Code = CKeyEvent::KEY_s;	break;
			case XK_space:	spEvent->m_Code = CKeyEvent::KEY_SPACE;	break;
			case XK_Left:	spEvent->m_Code = CKeyEvent::KEY_LEFT;	break;
			case XK_Right:	spEvent->m_Code = CKeyEvent::KEY_RIGHT;	break;
			case XK_Up:		spEvent->m_Code = CKeyEvent::KEY_UP;	break;
			case XK_Down:	spEvent->m_Code = CKeyEvent::KEY_DOWN;	break;
			case XK_Escape:	spEvent->m_Code = CKeyEvent::KEY_Esc;	break;
        }

		spCEvent e = spEvent;
		m_EventQueue.push( e );
    }
}

/*
*/
/*bool CUnixGL::checkKeyEvent(KeyEvent& event)
{
    XEvent xEvent;
    bool gotEvent = false;

    gotEvent = XCheckWindowEvent( m_pDisplay, m_Window, KeyPressMask | KeyReleaseMask, &xEvent );

    if( gotEvent )
    {
        if (xEvent.type == KeyPress)
        {
            event.pressed = true;
        }
        else if (xEvent.type == KeyRelease)
        {
            event.pressed = false;
        }

        KeySym keySymbol = XKeycodeToKeysym(m_pDisplay, xEvent.xkey.keycode, 0);
        switch (keySymbol)
        {
            case XK_a:
                event.keycode = KeyEvent::KEY_a;
                break;
            case XK_d:
                event.keycode = KeyEvent::KEY_d;
                break;
            case XK_f:
                event.keycode = KeyEvent::KEY_f;
                break;
            case XK_h:
                event.keycode = KeyEvent::KEY_h;
                break;
            case XK_q:
                event.keycode = KeyEvent::KEY_q;
                break;
            case XK_r:
                event.keycode = KeyEvent::KEY_r;
                break;
            case XK_v:
                event.keycode = KeyEvent::KEY_v;
                break;
            case XK_x:
                event.keycode = KeyEvent::KEY_x;
                break;
            case XK_y:
                event.keycode = KeyEvent::KEY_y;
                break;
            case XK_space:
                event.keycode = KeyEvent::KEY_SPACE;
                break;
            case XK_Left:
                event.keycode = KeyEvent::KEY_LEFT;
                break;
            case XK_Right:
                event.keycode = KeyEvent::KEY_RIGHT;
                break;
            case XK_Up:
                event.keycode = KeyEvent::KEY_UP;
                break;
            case XK_Down:
                event.keycode = KeyEvent::KEY_DOWN;
                break;
            case XK_F1:
                event.keycode = KeyEvent::KEY_F1;
                break;
            case XK_F2:
                event.keycode = KeyEvent::KEY_F2;
                break;
            case XK_F3:
                event.keycode = KeyEvent::KEY_F3;
                break;
            case XK_F4:
                event.keycode = KeyEvent::KEY_F4;
                break;
            case XK_F5:
                event.keycode = KeyEvent::KEY_F5;
                break;
            case XK_F6:
                event.keycode = KeyEvent::KEY_F6;
                break;
            case XK_F7:
                event.keycode = KeyEvent::KEY_F7;
                break;
            case XK_F8:
                event.keycode = KeyEvent::KEY_F8;
                break;
            case XK_F9:
                event.keycode = KeyEvent::KEY_F9;
                break;
            case XK_F10:
                event.keycode = KeyEvent::KEY_F10;
                break;
            case XK_F11:
                event.keycode = KeyEvent::KEY_F11;
                break;
            case XK_F12:
                event.keycode = KeyEvent::KEY_F12;
                break;
            default:
                event.keycode = KeyEvent::KEY_NONE;
        }
    }

    return gotEvent;
}*/

};

#endif
