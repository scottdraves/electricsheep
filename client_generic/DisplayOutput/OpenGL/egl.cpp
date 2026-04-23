#ifndef WIN32
#ifdef HAVE_WAYLAND

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <assert.h>
#include <iostream>
#include <string>
#include "Exception.h"
#include "Log.h"
#include "egl.h"

#include "egl.h"

namespace DisplayOutput {

CWaylandGL::CWaylandGL() : CDisplayOutput() {
  // Constructor initialization
}

CWaylandGL::~CWaylandGL() {
  // Cleanup
#ifdef HAVE_LIBDECOR
  if (m_LibdecorContext) {
    libdecor_unref(m_LibdecorContext);
  }
#endif
  if (m_EGLDisplay != EGL_NO_DISPLAY) {
    eglMakeCurrent(m_EGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);
    if (m_EGLContext != EGL_NO_CONTEXT) {
      eglDestroyContext(m_EGLDisplay, m_EGLContext);
    }
    if (m_EGLSurface != EGL_NO_SURFACE) {
      eglDestroySurface(m_EGLDisplay, m_EGLSurface);
    }
    eglTerminate(m_EGLDisplay);
  }
  if (m_EGLWindow) {
    wl_egl_window_destroy(m_EGLWindow);
  }
  if (m_Surface) {
    wl_surface_destroy(m_Surface);
  }
  if (m_pDisplay) {
    wl_display_disconnect(m_pDisplay);
  }
}

bool CWaylandGL::Initialize(const uint32 _width, const uint32 _height,
                            const bool _bFullscreen) {
  m_Width = m_WidthFS = _width;
  m_Height = m_HeightFS = _height;
  fprintf(stderr, "CWaylandGL()\n");
                              fprintf(stderr, "initial width, height: %d, %d\n", _width, _height);

  // create xkb context
  m_XkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  assert(m_XkbContext);

  // Connect to the Wayland display
  m_pDisplay = wl_display_connect(NULL);
  display_fd = wl_display_get_fd(m_pDisplay);
  disp_fd.events = POLLIN;
  disp_fd.fd = display_fd;
  assert(m_pDisplay);

  // Create a Wayland registry
  struct wl_registry *registry = wl_display_get_registry(m_pDisplay);
  assert(registry);

  // Add listeners to the registry to handle global events

  wl_registry_add_listener(registry, &registry_listener, this);
  // wl_display_dispatch(m_pDisplay);
  wl_display_roundtrip(m_pDisplay);
  wl_display_roundtrip(m_pDisplay);

  assert(m_Compositor);

  // Initialize EGL
  EGLint count, size, numConfigs;
  EGLConfig *configs;
  EGLint configAttribs[] = {EGL_SURFACE_TYPE,
                            EGL_WINDOW_BIT,
                            EGL_RED_SIZE,
                            8,
                            EGL_GREEN_SIZE,
                            8,
                            EGL_BLUE_SIZE,
                            8,
                            EGL_DEPTH_SIZE,
                            24,
                            EGL_ALPHA_SIZE,
                            8,
                            EGL_RENDERABLE_TYPE,
                            EGL_OPENGL_BIT,
                            EGL_NONE};
  EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  m_EGLDisplay = eglGetDisplay((EGLNativeDisplayType)m_pDisplay);
  assert(m_EGLDisplay);
  if (m_EGLDisplay == EGL_NO_DISPLAY) {
    fprintf(stderr, "Can't create egl display\n");
    return false;
  }

  if (!eglInitialize(m_EGLDisplay, NULL, NULL)) {
    fprintf(stderr, "eglInitialize failed with error: 0x%x\n", eglGetError());
    return false;
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    fprintf(stderr, "eglBindAPI failed with error: 0x%x\n", eglGetError());
    return false;
  }

  if (!eglGetConfigs(m_EGLDisplay, NULL, 0, &count) || count < 1) {
    fprintf(stderr, "Failed to get configs\n");
    return false;
  }

  configs = (EGLConfig *)calloc(count, sizeof(EGLConfig));
  if (!configs) {
    fprintf(stderr, "Failed to allocate config list");
    return false;
  }

  if (!eglChooseConfig(m_EGLDisplay, configAttribs, configs, count,
                       &numConfigs) ||
      numConfigs == 0) {
    fprintf(stderr, "eglChooseConfig failed with error: 0x%x\n", eglGetError());
    return false;
  }
  m_EGLConfig = configs[0];
  m_EGLContext = eglCreateContext(m_EGLDisplay, m_EGLConfig, EGL_NO_CONTEXT,
                                  contextAttribs);
  if (m_EGLContext == EGL_NO_CONTEXT) {
    fprintf(stderr, "eglCreateContext failed with error: 0x%x\n",
            eglGetError());
    return false;
  }

  m_Surface = wl_compositor_create_surface(m_Compositor);
  assert(m_Surface);
  fprintf(stderr, "Created surface\n");

  const char *is_background = getenv("ELECTRICSHEEP_BACKGROUND");
  const char *force_dec_man = getenv("ELECTRICSHEEP_FORCE_SSD");
  const char *force_libdecor = getenv("ELECTRICSHEEP_FORCE_CSD");
  const char *force_layer_shell = getenv("ELECTRICSHEEP_FORCE_LAYER_SHELL");

#ifndef HAVE_LIBDECOR
  if (force_libdecor) {
    fprintf(stderr, "Can not force client side (libdecor) decorations when electricsheep is not compiled with libdecor");
    return false;
  }
#endif
  if (is_background) {
    m_Background = true;
  }

  fprintf(stderr, "is background: %d\n",m_Background);
  if (!m_Background && !force_layer_shell) { // normal window
    if ((m_DecorationManager || force_dec_man) && !force_libdecor) { // compositor knows xdg-decoration, use xdg-shell
      // and server side decor (or decoration manager was forced)
      using_csd = false;
      configured = false;
      fprintf(stderr, "Using xdg-shell and server side decor\n");
      createEGLWindow(_width, _height);
      assert(m_XdgWmBase);
      m_XdgSurface = xdg_wm_base_get_xdg_surface(m_XdgWmBase, m_Surface);
      assert(m_XdgSurface);
      xdg_surface_add_listener(m_XdgSurface, &xdg_surface_listener, this);
      fprintf(stderr, "Created xdgsurface\n");
      m_XdgToplevel = xdg_surface_get_toplevel(m_XdgSurface);
      assert(m_XdgToplevel);
      xdg_toplevel_add_listener(m_XdgToplevel, &xdg_toplevel_listener, this);
      fprintf(stderr, "Created toplevel\n");
      m_Decoration = zxdg_decoration_manager_v1_get_toplevel_decoration(
          m_DecorationManager, m_XdgToplevel);
      zxdg_toplevel_decoration_v1_set_mode(
          m_Decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
      xdg_surface_set_window_geometry(m_XdgSurface, 0, 0, _width, _height);
    }
#ifdef HAVE_LIBDECOR
    else { // compositor does not know xdg-decoration, use libdecor and client
           // side decor
      fprintf(stderr, "Using libdecor\n");
      using_csd = true;
      configured = false;
      createEGLWindow(_width, _height);
      m_LibdecorFloatingWidth = m_WidthFS;
      m_LibdecorFloatingHeight = m_HeightFS;
      m_LibdecorContext = libdecor_new(m_pDisplay, &libdecor_interface);
      m_LibdecorFrame = libdecor_decorate(m_LibdecorContext, m_Surface,
                                          &frame_interface, this);
      libdecor_frame_map(m_LibdecorFrame);
      while (!configured) {
        if (libdecor_dispatch(m_LibdecorContext, 0) < 0) {
          fprintf(stderr, "Failed to dispatch libdecor\n");
          return false;
        }
      }
    }
#else
    else {
      fprintf(stderr, "Compositor does not support xdg-decoration\n");
      fprintf(stderr,
              "and electricsheep is compiled without libdecor support.\n");
      fprintf(stderr, "Still trying basic output without title bar.\n");
      assert(m_XdgWmBase);
      using_csd = false;
      configured = false;
      createEGLWindow(_width, _height);
      m_XdgSurface = xdg_wm_base_get_xdg_surface(m_XdgWmBase, m_Surface);
      assert(m_XdgSurface);
      xdg_surface_add_listener(m_XdgSurface, &xdg_surface_listener, this);
      fprintf(stderr, "Created xdgsurface\n");
      m_XdgToplevel = xdg_surface_get_toplevel(m_XdgSurface);
      assert(m_XdgToplevel);
      xdg_toplevel_add_listener(m_XdgToplevel, &xdg_toplevel_listener, this);
      fprintf(stderr, "Created toplevel\n");
      xdg_surface_set_window_geometry(m_XdgSurface, 0, 0, _width, _height);
    }
#endif
  } else { // wl_layer_shell
    if (!m_WlrLayerShell) {
      fprintf(stderr, "Compositor does not support layer shell\n");
      return false;
    }
    using_csd = false;
    configured = false;
    layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        m_WlrLayerShell, m_Surface, m_Output,
        ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wallpaper");
    zwlr_layer_surface_v1_set_size(layer_surface, 0, 0);
    zwlr_layer_surface_v1_set_anchor(layer_surface,
        ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
        ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
    zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                       this);
    fprintf(stderr, "Created wlr-layer-shell\n");
  }

  wl_surface_commit(m_Surface);
  wl_display_roundtrip(m_pDisplay);

  // setFullScreen(_bFullscreen);

  free(configs);

  return true;
}

void CWaylandGL::Title(const std::string &_title) {
  // Set window title
  if (!m_Background) {
    if (!using_csd) {
      xdg_toplevel_set_title(m_XdgToplevel, _title.c_str());
      xdg_toplevel_set_app_id(m_XdgToplevel, _title.c_str());
    }
#ifdef HAVE_LIBDECOR
    else {
      libdecor_frame_set_title(m_LibdecorFrame, _title.c_str());
      libdecor_frame_set_app_id(m_LibdecorFrame, _title.c_str());
    }
#endif
  }
}

void CWaylandGL::setFullScreen(bool enabled) {
  // Fullscreen setup
  fprintf(stderr, "Setting fullscreen: %d\n", enabled);
  if (!m_Background) {
    if (enabled) {
#ifdef HAVE_LIBDECOR
      if (using_csd)
        libdecor_frame_set_fullscreen(m_LibdecorFrame, m_Output);
      else
#endif
        xdg_toplevel_set_fullscreen(m_XdgToplevel, m_Output);
      m_FullScreen = true;
    } else {
#ifdef HAVE_LIBDECOR
      if (using_csd)
        libdecor_frame_unset_fullscreen(m_LibdecorFrame);
      else
#endif
        xdg_toplevel_unset_fullscreen(m_XdgToplevel);
      m_FullScreen = false;
    }
  }
}

void CWaylandGL::Update() { // nothing to do
  if (poll(&disp_fd, 1, 0) == -1 && errno != EINTR) {
    fprintf(stderr, "Poll error\n");
    running = false;
  }
  if (disp_fd.revents & POLLIN) {
    // handle inputs before hang-up message
    if (wl_display_dispatch(m_pDisplay) == -1 && errno != EINTR) {
      running = false;
    }
  }
  if (disp_fd.revents & POLLHUP) {
    // compositor has closed the connection
    fprintf(stderr, "compositor has closed the connection\n");
    running = false;
  }
  if (disp_fd.revents & POLLERR) {
    // error condition
    fprintf(stderr, "Display fd error condition\n");
    running = false;
  }

  if (!running) {
    exit(EXIT_SUCCESS);
  }
}

void CWaylandGL::SwapBuffers() {
#ifdef HAVE_LIBDECOR
  if (using_csd) {
    if (libdecor_dispatch(m_LibdecorContext, 0) < 0) {
      fprintf(stderr, "libdecor dispatch failed\n");
    }
  }
#endif
  if (configured) {
    if (!eglSwapBuffers(m_EGLDisplay, m_EGLSurface)) {
      fprintf(stderr, "Swapping buffers failed\n");
    }
  }
}

bool CWaylandGL::createEGLWindow(const uint32 _width, const uint32 _height) {
    m_EGLWindow = wl_egl_window_create(m_Surface, _width, _height);
    if (!m_EGLWindow) {
      fprintf(stderr, "wl_egl_window_create failed\n");
      return false;
    }

    EGLint surfaceAttribs[] = {EGL_RENDER_BUFFER, EGL_BACK_BUFFER, EGL_NONE};
    m_EGLSurface =
      eglCreateWindowSurface(m_EGLDisplay, m_EGLConfig,
                             (EGLNativeWindowType)m_EGLWindow, surfaceAttribs);
    if (m_EGLSurface == EGL_NO_SURFACE) {
      fprintf(stderr, "eglCreateWindowSurface failed with error: 0x%x\n",
              eglGetError());
      return false;
    }

    if (!eglMakeCurrent(m_EGLDisplay, m_EGLSurface, m_EGLSurface, m_EGLContext)) {
      fprintf(stderr, "eglMakeCurrent failed with error: 0x%x\n", eglGetError());
      return false;
    }

    /* Ensure that buffer swaps for egl_surface are not synchronized
     * to the compositor, as this would result in blocking and round-robin
     * updates when there are multiple outputs */
    if (!eglSwapInterval(m_EGLDisplay, 0)) {
      fprintf(stderr, "Failed to set swap interval\n");
      return false;
    }
    return true;
  }

void CWaylandGL::handleKeyboard(xkb_keysym_t keysym, uint32_t codepoint,
                                enum wl_keyboard_key_state key_state) {

  fprintf(stderr, "Key event: %d\n", keysym);
  fprintf(stderr, "Codepoint: %d\n", codepoint);
  fprintf(stderr, "Pushed/Released: %d\n", key_state);
  CKeyEvent *spEvent = new CKeyEvent();

  if (key_state == WL_KEYBOARD_KEY_STATE_PRESSED)
    spEvent->m_bPressed = true;
  else if (key_state == WL_KEYBOARD_KEY_STATE_RELEASED)
    spEvent->m_bPressed = false;

  switch (keysym) {
  case XKB_KEY_F1:
    spEvent->m_Code = CKeyEvent::KEY_F1;
    break;
  case XKB_KEY_F2:
    spEvent->m_Code = CKeyEvent::KEY_F2;
    break;
  case XKB_KEY_F3:
    spEvent->m_Code = CKeyEvent::KEY_F3;
    break;
  case XKB_KEY_F4:
    spEvent->m_Code = CKeyEvent::KEY_F4;
    break;
  case XKB_KEY_F8:
    spEvent->m_Code = CKeyEvent::KEY_F8;
    break;
  case XKB_KEY_f:
    spEvent->m_Code = CKeyEvent::KEY_F;
    break;
  case XKB_KEY_s:
    spEvent->m_Code = CKeyEvent::KEY_s;
    break;
  case XKB_KEY_space:
    spEvent->m_Code = CKeyEvent::KEY_SPACE;
    break;
  case XKB_KEY_Left:
    spEvent->m_Code = CKeyEvent::KEY_LEFT;
    break;
  case XKB_KEY_Right:
    spEvent->m_Code = CKeyEvent::KEY_RIGHT;
    break;
  case XKB_KEY_Up:
    spEvent->m_Code = CKeyEvent::KEY_UP;
    break;
  case XKB_KEY_Down:
    spEvent->m_Code = CKeyEvent::KEY_DOWN;
    break;
  case XKB_KEY_Escape:
    spEvent->m_Code = CKeyEvent::KEY_Esc;
    break;
  }

  spCEvent e = spEvent;
  m_EventQueue.push(e);
}

} // namespace DisplayOutput

#endif
#endif
