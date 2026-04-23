#ifndef EGL_VIDEO_OUTPUT_H
#define EGL_VIDEO_OUTPUT_H

#ifndef WIN32
#ifdef HAVE_WAYLAND

#ifdef _DisplayGL_H_
#error "DisplayGL.h included before egl.h!"
#endif

#include <poll.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <sys/mman.h>
#include <xkbcommon/xkbcommon.h>
#ifdef HAVE_LIBDECOR
  #include <libdecor.h>
#endif

#ifndef LINUX_GNU
  #include "GLee.h"
#else
  #include <GLee.h>
#endif

#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
/* HACK: unfortunately the wlr-layer-shell header file uses the namespace
 * keyword as a variable name, which is a reserved keyword in C++. To work
 * around this, we temporarily rename the namespace keyword to namespace_.
 */
#ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wkeyword-macro"
#endif
  #define namespace namespace_
#ifdef __clang__
  #pragma clang diagnostic pop
#endif
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#undef namespace

#include "DisplayOutput.h"

namespace DisplayOutput {

class CWaylandGL : public CDisplayOutput {
  bool running = true;
  bool using_csd = false;
  bool configured = false;
  // display fd to react to changes of display
  int display_fd = 0;
  pollfd disp_fd;
  // wayland
  wl_display *m_pDisplay = nullptr;
  wl_compositor *m_Compositor = nullptr;
  wl_surface *m_Surface = nullptr;
  wl_output *m_Output = nullptr;
  // seat
  wl_seat *m_Seat = nullptr;
  wl_pointer *pointer = nullptr;
  wl_touch *touch = nullptr;
  wl_keyboard *keyboard = nullptr;
  bool caps_lock = false;
  bool control = false;
  xkb_state *m_XkbState = nullptr;
  xkb_context *m_XkbContext = nullptr;
  xkb_keymap *m_XkbKeymap = nullptr;
  // xdg
  xdg_wm_base *m_XdgWmBase = nullptr;
  xdg_surface *m_XdgSurface = nullptr;
  xdg_toplevel *m_XdgToplevel = nullptr;
  zxdg_decoration_manager_v1 *m_DecorationManager = nullptr;
  zxdg_toplevel_decoration_v1 *m_Decoration = nullptr;
  // wlr layer shell
  zwlr_layer_shell_v1 *m_WlrLayerShell = nullptr;
  zwlr_layer_surface_v1 *layer_surface = nullptr;
  // egl
  wl_egl_window *m_EGLWindow = nullptr;
  EGLDisplay m_EGLDisplay = nullptr;
  EGLContext m_EGLContext = nullptr;
  EGLSurface m_EGLSurface = nullptr;
  EGLConfig m_EGLConfig = nullptr;

#ifdef HAVE_LIBDECOR
  libdecor *m_LibdecorContext = nullptr;
  libdecor_frame *m_LibdecorFrame = nullptr;
  int m_LibdecorContentWidth = 0;
  int m_LibdecorContentHeight = 0;
  int m_LibdecorFloatingWidth = 0;
  int m_LibdecorFloatingHeight = 0;
#endif

  bool m_FullScreen = false;
  bool m_Background = false;

  uint32 m_WidthFS = 0;
  uint32 m_HeightFS = 0;

  void setFullScreen(bool enabled);
  void handleKeyboard(xkb_keysym_t keysym, uint32_t codepoint,
                      enum wl_keyboard_key_state key_state);
  bool createEGLWindow(const uint32 _width, const uint32 _height);
// wayland handling stuff starts here
#ifdef HAVE_LIBDECOR
  static void libdecor_my_frame_configure(struct libdecor_frame *frame,
                              struct libdecor_configuration *configuration,
                              void *user_data) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(user_data);
    struct libdecor_state *state;
    int width, height;

    if (!libdecor_configuration_get_content_size(configuration, frame, &width,
                                                 &height)) {
      width = waylandGL->m_LibdecorFloatingWidth;
      height = waylandGL->m_LibdecorFloatingHeight;
    }

    waylandGL->m_LibdecorContentWidth = width;
    waylandGL->m_LibdecorContentHeight = height;

    wl_egl_window_resize(waylandGL->m_EGLWindow,
                         waylandGL->m_LibdecorContentWidth,
                         waylandGL->m_LibdecorContentHeight, 0, 0);
    glViewport(0, 0, waylandGL->m_LibdecorContentWidth,
               waylandGL->m_LibdecorContentHeight);

    state = libdecor_state_new(width, height);
    libdecor_frame_commit(frame, state, configuration);
    libdecor_state_free(state);

    /* store floating dimensions */
    if (libdecor_frame_is_floating(waylandGL->m_LibdecorFrame)) {
      waylandGL->m_LibdecorFloatingWidth = width;
      waylandGL->m_LibdecorFloatingHeight = height;
    }

    waylandGL->configured = true;
  }

  static void libdecor_my_frame_close(struct libdecor_frame *frame, void *user_data) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(user_data);

    waylandGL->m_bClosed = true;
  }

  static void libdecor_my_frame_commit(struct libdecor_frame *frame, void *user_data) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(user_data);
    eglSwapBuffers(waylandGL->m_EGLDisplay, waylandGL->m_EGLSurface);
  }

  struct libdecor_frame_interface frame_interface = {
      libdecor_my_frame_configure,
      libdecor_my_frame_close,
      libdecor_my_frame_commit,
  };

  static void libdecor_error(struct libdecor *context,
                             enum libdecor_error error, const char *message) {
    fprintf(stderr, "Caught error (%d): %s\n", error, message);
    exit(EXIT_FAILURE);
  }

  struct libdecor_interface libdecor_interface = {
      libdecor_error,
  };
#endif

  static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                               uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
  }

  const struct xdg_wm_base_listener xdg_wm_base_listener = {
      .ping = xdg_wm_base_ping,
  };

  static void xdg_toplevel_configure_handler(void *data,
                                             struct xdg_toplevel *xdg_toplevel,
                                             int32_t width, int32_t height,
                                             struct wl_array *states) {
    fprintf(stderr, "configure: %dx%d\n", width, height);
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    if (width == 0 || height == 0) {
      /* Compositor is deferring to us */
      return;
    }
    waylandGL->m_WidthFS = width;
    waylandGL->m_HeightFS = height;
    wl_egl_window_resize(waylandGL->m_EGLWindow,
                         waylandGL->m_WidthFS,
                         waylandGL->m_HeightFS, 0, 0);
    glViewport(0, 0, waylandGL->m_WidthFS,
               waylandGL->m_HeightFS);
  }

  static void xdg_toplevel_close_handler(void *data,
                                         struct xdg_toplevel *xdg_toplevel) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    waylandGL->m_bClosed = true;
  }

  const struct xdg_toplevel_listener xdg_toplevel_listener = {
      .configure = xdg_toplevel_configure_handler,
      .close = xdg_toplevel_close_handler};

  static void xdg_surface_configure_handler(void *data,
                                            struct xdg_surface *xdg_surface,
                                            uint32_t serial) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    xdg_surface_ack_configure(xdg_surface, serial);
    waylandGL->configured = true;
  }

  const struct xdg_surface_listener xdg_surface_listener = {
      .configure = xdg_surface_configure_handler};

  static void zwlr_layer_surface_configure_handler(
      void *data, struct zwlr_layer_surface_v1 *wlr_surface,
      uint32_t serial, uint32_t width, uint32_t height) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    fprintf(stderr, "width: %d, height: %d\n", width, height);
    glViewport(0, 0, width, height);
    // wlr_layer_shell requires late initialization because with
    // wl_egl_window_resize() swaylock_plugin still detects the old
    // size as surface size and refuses to display electricsheep
    if (!waylandGL->createEGLWindow(width, height)) {
      fprintf(stderr, "could not create egl window\n");
    }
    zwlr_layer_surface_v1_ack_configure(waylandGL->layer_surface, serial);
    waylandGL->configured = true;
    fprintf(stderr, "wlr_layer_surface configured\n");
  }

  static void zwlr_layer_surface_close_handler(
      void *data, struct zwlr_layer_surface_v1 *wlr_surface) {
        CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
        if (waylandGL->m_EGLWindow) {
          eglDestroyContext(waylandGL->m_EGLDisplay, waylandGL->m_EGLContext);
          eglDestroySurface(waylandGL->m_EGLDisplay, waylandGL->m_EGLSurface);
          wl_egl_window_destroy(waylandGL->m_EGLWindow);
        }
        if (waylandGL->m_Surface) {
          wl_surface_destroy(waylandGL->m_Surface);
        }
        if (waylandGL->layer_surface) {
          zwlr_layer_surface_v1_destroy(waylandGL->layer_surface);
        }
  }

  const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = zwlr_layer_surface_configure_handler,
    .closed = zwlr_layer_surface_close_handler
  };

  static void keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t format, int32_t fd, uint32_t size) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
      close(fd);
      fprintf(stderr, "Unsupported keymap format\n");
      return;
    }
    char *map_shm = (char *)mmap(NULL, size - 1, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_shm == MAP_FAILED) {
      close(fd);
      fprintf(stderr, "Unable to initialize keymap shm\n");
      return;
    }

    struct xkb_keymap *keymap = xkb_keymap_new_from_buffer(
        waylandGL->m_XkbContext, map_shm, size - 1, XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map_shm, size - 1);
    close(fd);
    assert(keymap);
    struct xkb_state *xkb_state = xkb_state_new(keymap);
    assert(xkb_state);
    xkb_keymap_unref(waylandGL->m_XkbKeymap);
    xkb_state_unref(waylandGL->m_XkbState);
    waylandGL->m_XkbKeymap = keymap;
    waylandGL->m_XkbState = xkb_state;
  }

  static void keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                             uint32_t serial, struct wl_surface *surface,
                             struct wl_array *keys) {}

  static void keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                             uint32_t serial, struct wl_surface *surface) {}

  static void keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, uint32_t time, uint32_t key,
                           uint32_t _key_state) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    enum wl_keyboard_key_state key_state = (wl_keyboard_key_state)_key_state;
    xkb_keysym_t sym =
        xkb_state_key_get_one_sym(waylandGL->m_XkbState, key + 8);
    uint32_t keycode = key_state == WL_KEYBOARD_KEY_STATE_PRESSED ? key + 8 : 0;
    uint32_t codepoint =
        xkb_state_key_get_utf32(waylandGL->m_XkbState, keycode);
    waylandGL->handleKeyboard(sym, codepoint, key_state);
  }

  static void keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                                 uint32_t serial, uint32_t mods_depressed,
                                 uint32_t mods_latched, uint32_t mods_locked,
                                 uint32_t group) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    if (waylandGL->m_XkbState == NULL) {
      return;
    }

    int layout_same = xkb_state_layout_index_is_active(
        waylandGL->m_XkbState, group, XKB_STATE_LAYOUT_EFFECTIVE);
    // if (!layout_same) {
    // damage_state(state);
    // }
    xkb_state_update_mask(waylandGL->m_XkbState, mods_depressed, mods_latched,
                          mods_locked, 0, 0, group);
    int caps_lock = xkb_state_mod_name_is_active(
        waylandGL->m_XkbState, XKB_MOD_NAME_CAPS, XKB_STATE_MODS_LOCKED);
    if (caps_lock != waylandGL->caps_lock) {
      waylandGL->caps_lock = caps_lock;
    }
    waylandGL->control = xkb_state_mod_name_is_active(
        waylandGL->m_XkbState, XKB_MOD_NAME_CTRL,
        (xkb_state_component)(XKB_STATE_MODS_DEPRESSED |
                              XKB_STATE_MODS_LATCHED));
  }

  static void keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                   int32_t rate, int32_t delay) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    // if (rate <= 0) {
    //   waylandGL->repeat_period_ms = -1;
    // } else {
    //   // Keys per second -> milliseconds between keys
    //   waylandGL->repeat_period_ms = 1000 / rate;
    // }
    // waylandGL->repeat_delay_ms = delay;
  }
  const struct wl_keyboard_listener keyboard_listener = {
      .keymap = keyboard_keymap,
      .enter = keyboard_enter,
      .leave = keyboard_leave,
      .key = keyboard_key,
      .modifiers = keyboard_modifiers,
      .repeat_info = keyboard_repeat_info,
  };

  static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
                               uint32_t serial, struct wl_surface *surface,
                               wl_fixed_t surface_x, wl_fixed_t surface_y) {
//    wl_pointer_set_cursor(wl_pointer, serial, NULL, 0, 0);
  }

  static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                               uint32_t serial, struct wl_surface *surface) {
    // Who cares
  }

  static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
                                uint32_t time, wl_fixed_t surface_x,
                                wl_fixed_t surface_y) {}

  static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                                uint32_t serial, uint32_t time, uint32_t button,
                                uint32_t state) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    static uint32_t last_time = 0;
    fprintf(stderr, "button");
    // double click toggles fullscreen
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
      if (time - last_time < 500) {
        fprintf(stderr, "button2");
        waylandGL->setFullScreen((waylandGL->m_FullScreen) ? false : true);
      }
    }
    last_time = time;
  }

  static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                              uint32_t time, uint32_t axis, wl_fixed_t value) {}

  static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer) {}

  static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                                     uint32_t axis_source) {}

  static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                                   uint32_t time, uint32_t axis) {}

  static void wl_pointer_axis_discrete(void *data,
                                       struct wl_pointer *wl_pointer,
                                       uint32_t axis, int32_t discrete) {}
  const struct wl_pointer_listener pointer_listener = {
      .enter = wl_pointer_enter,
      .leave = wl_pointer_leave,
      .motion = wl_pointer_motion,
      .button = wl_pointer_button,
      .axis = wl_pointer_axis,
      .frame = wl_pointer_frame,
      .axis_source = wl_pointer_axis_source,
      .axis_stop = wl_pointer_axis_stop,
      .axis_discrete = wl_pointer_axis_discrete,
  };

static void
wl_touch_down(void *data, struct wl_touch *wl_touch, uint32_t serial,
                                uint32_t time, struct wl_surface *surface, int32_t id,
                                wl_fixed_t x, wl_fixed_t y) {
  CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
  static uint32_t last_time = 0;
  // double click toggles fullscreen
  if (time - last_time < 500) {
    waylandGL->setFullScreen((waylandGL->m_FullScreen) ? false : true);
  }
  last_time = time;
}

const struct wl_touch_listener touch_listener = {
           .down = wl_touch_down,
             .up = nullptr,
             .motion = nullptr,
             .frame = nullptr,
             .cancel = nullptr,
             .shape = nullptr,
             .orientation = nullptr,
};

  static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
                                       uint32_t caps) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    if (waylandGL->pointer) {
      wl_pointer_release(waylandGL->pointer);
      waylandGL->pointer = nullptr;
    }
    if (waylandGL->touch) {
      wl_touch_release(waylandGL->touch);
      waylandGL->touch = nullptr;
    }
    if (waylandGL->keyboard) {
      wl_keyboard_release(waylandGL->keyboard);
      waylandGL->keyboard = nullptr;
    }
    if ((caps & WL_SEAT_CAPABILITY_POINTER)) {
      waylandGL->pointer = wl_seat_get_pointer(wl_seat);
      wl_pointer_add_listener(waylandGL->pointer, &waylandGL->pointer_listener,
                              waylandGL);
    }
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
      waylandGL->keyboard = wl_seat_get_keyboard(wl_seat);
      wl_keyboard_add_listener(waylandGL->keyboard,
                               &waylandGL->keyboard_listener, waylandGL);
    }
    if ((caps & WL_SEAT_CAPABILITY_TOUCH)) {
      waylandGL->touch = wl_seat_get_touch(wl_seat);
      wl_touch_add_listener(waylandGL->touch,
                               &waylandGL->touch_listener, waylandGL);
    }
  }

  static void seat_handle_name(void *data, struct wl_seat *wl_seat,
                               const char *name) {
    // Who cares
  }
  const struct wl_seat_listener seat_listener = {
      .capabilities = seat_handle_capabilities,
      .name = seat_handle_name,
  };

  static void registry_handle_global(void *data, struct wl_registry *registry,
                                     uint32_t name, const char *interface,
                                     uint32_t version) {
    CWaylandGL *waylandGL = static_cast<CWaylandGL *>(data);
    // fprintf(stderr, "Got a registry event for %s id %d\n", interface, name);
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
      waylandGL->m_Compositor = (struct wl_compositor *)wl_registry_bind(
          registry, name, &wl_compositor_interface, 1);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
      waylandGL->m_XdgWmBase = (struct xdg_wm_base *)wl_registry_bind(
          registry, name, &xdg_wm_base_interface, 1);
      xdg_wm_base_add_listener(waylandGL->m_XdgWmBase,
                               &waylandGL->xdg_wm_base_listener, waylandGL);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
      waylandGL->m_Output = (struct wl_output *)wl_registry_bind(
          registry, name, &wl_output_interface, 1);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
      waylandGL->m_WlrLayerShell =
          (struct zwlr_layer_shell_v1 *)wl_registry_bind(
              registry, name, &zwlr_layer_shell_v1_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
      waylandGL->m_Seat = (struct wl_seat *)wl_registry_bind(
          registry, name, &wl_seat_interface, 7);
      wl_seat_add_listener(waylandGL->m_Seat, &waylandGL->seat_listener,
                           waylandGL);
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) ==
               0) {
      waylandGL->m_DecorationManager =
          (struct zxdg_decoration_manager_v1 *)wl_registry_bind(
              registry, name, &zxdg_decoration_manager_v1_interface, 1);
    }
  }
  const struct wl_registry_listener registry_listener = {registry_handle_global,
                                                         nullptr};
  // wayland handling stuff ends here

public:
  CWaylandGL();
  virtual ~CWaylandGL();

  static const char *Description() { return "Wayland OpenGL display"; };

  virtual bool Initialize(const uint32 _width, const uint32 _height,
                          const bool _bFullscreen);
  virtual void Title(const std::string &_title);
  virtual void Update();

  // void request_next_frame();
  void SwapBuffers();
};

#if defined(HAVE_WAYLAND) && !defined(HAVE_X11)
/* See player.cpp
- If we have wayland and X11, we decide at runtime to load CWaylandGL in
player.cpp
- If we have wayland and no X11, typedef CWaylandGL to CDisplayGL and egl.h gets
included in DisplayGL.h
- If we have X11 and no wayland, typedef CDisplayGL to CUnixGL and glx.h gets
included in DisplayGL.h
*/
typedef CWaylandGL CDisplayGL;
#endif

} // namespace DisplayOutput

#endif
#endif
#endif
