#ifndef _DISPLAY_OUTPUT_H
#define	_DISPLAY_OUTPUT_H

#include	<queue>
#include	"base.h"
#include	"SmartPtr.h"
#include	"linkpool.h"
#ifdef MAC
#undef Random
#include	<OpenGL/OpenGL.h>
#endif

namespace	DisplayOutput
{

/*
	CEvent.
	Event interface.
*/
class	CEvent
{
	public:
			enum eEventType
			{
				Event_KEY,
				Event_Mouse,
				Event_Power,
				Event_NONE
			};
    
            virtual ~CEvent() {}

			virtual	eEventType	Type()	{	return( Event_NONE );	};

			POOLED( CEvent, Memory::CLinkPool );
};

MakeSmartPointers( CEvent );

/*
	CKeyEvent.
	Keyboard event.
*/
class	CKeyEvent : public CEvent
{
	public:
			enum eKeyCode
			{
				KEY_TAB,
				KEY_LALT,
				KEY_MENU,
				KEY_CTRL,
				KEY_F,
				KEY_s,
				KEY_SPACE,
				KEY_LEFT,
				KEY_RIGHT,
				KEY_UP,
				KEY_DOWN,
				KEY_F1,
				KEY_F2,
				KEY_F3,
				KEY_F4,
				KEY_F5,
				KEY_F6,
				KEY_F7,
				KEY_F8,
				KEY_F9,
				KEY_F10,
				KEY_F11,
				KEY_F12,
				KEY_Esc,
				KEY_NONE
			};

			CKeyEvent() : m_bPressed( true ), m_Code( KEY_NONE )	{}
    
            virtual ~CKeyEvent() {}
    
			virtual eEventType	Type()	{	return( CEvent::Event_KEY );	};
			bool    	m_bPressed;
			eKeyCode	m_Code;

			POOLED( CKeyEvent, Memory::CLinkPool );
};

MakeSmartPointers( CKeyEvent );


/*
	CMouseEvent.
	Mouse event.
*/
class	CMouseEvent : public CEvent
{
	public:
			enum eMouseCode
			{
				Mouse_LEFT,
				Mouse_RIGHT,
				Mouse_MOVE,
				Mouse_NONE
			};

			CMouseEvent() : m_Code( Mouse_NONE )	{}
    
            virtual ~CMouseEvent() {}
    
			virtual eEventType	Type()	{	return( CEvent::Event_Mouse );	};
			eMouseCode	m_Code;

			int32	m_X;
			int32	m_Y;

			POOLED( CMouseEvent, Memory::CLinkPool );
};

MakeSmartPointers( CMouseEvent );



/*
	CPowerEvent.
	Power broadcast event.
*/
class	CPowerEvent : public CEvent
{
	public:
			CPowerEvent()	{}
    
            virtual ~CPowerEvent() {}

            virtual eEventType	Type()	{	return( CEvent::Event_Power );	};
};

MakeSmartPointers( CPowerEvent );

/*
	CDisplayOutput.
	Base class.

	Note, it's up to constructor(or Initialize()), to set width/height.
*/
class	CDisplayOutput
{
	protected:

		int32	m_XPosition;
		int32	m_YPosition;
		uint32	m_Width;
		uint32	m_Height;
		bool    m_bFullScreen;
		bool    m_bVSync;
		bool	m_bClosed;

		static std::queue<spCEvent>	m_EventQueue;

	public:
			CDisplayOutput();
			virtual ~CDisplayOutput();

#ifdef	WIN32
			virtual bool Initialize( HWND _hWnd, bool _bPreview ) = PureVirtual;
			virtual HWND Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen ) = PureVirtual;
			virtual HWND WindowHandle( void ) = PureVirtual;
			virtual DWORD GetNumMonitors() { return 1; }
#else
#ifdef MAC
			virtual bool Initialize( CGLContextObj _glContext, bool _bPreview ) = PureVirtual;
			virtual void SetContext( CGLContextObj glContext ) = PureVirtual;
			virtual CGLContextObj GetContext( void ) = PureVirtual;
			virtual void ForceWidthAndHeight( uint32 _width, uint32 _height ) = PureVirtual;
#else
			virtual bool Initialize( const uint32 _width, const uint32 _height, const bool _bFullscreen ) = PureVirtual;
#endif
#endif

			//
			virtual void Title( const std::string &_title) = PureVirtual;
			virtual void Update() = PureVirtual;
			virtual void SwapBuffers() = PureVirtual;

			bool	GetEvent( spCEvent &_event );
			void	AppendEvent( spCEvent _event );
			void	ClearEvents();

			virtual bool HasShaders() { return false; };
			uint32	Width()		
			{	
				return( m_Width );	    
			};
			uint32	Height()	
			{	
				return( m_Height );	    
			};
			fp4		Aspect()	{	return( (fp4)m_Height / (fp4)m_Width );	};
			bool    Closed()    {   return( m_bClosed );    };
			void	Close()		{	m_bClosed = true;		};
};

MakeSmartPointers( CDisplayOutput );

};

#endif
