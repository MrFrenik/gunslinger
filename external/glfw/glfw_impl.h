#ifndef __GLFW_IMPL_H__
#define __GLFW_IMPL_H__

#include "glfw3.h"

#ifdef GLFW_IMPL

#if defined(_WIN32)
    #define _GLFW_WIN32
#endif
#if defined(__linux__)
    #if !defined(_GLFW_WAYLAND)     // Required for Wayland windowing
        #define _GLFW_X11
    #endif
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    #define _GLFW_X11
#endif
#if defined(__APPLE__)
    #define _GLFW_COCOA
    #define _GLFW_USE_MENUBAR       // To create and populate the menu bar when the first window is created
    #define _GLFW_USE_RETINA        // To have windows use the full resolution of Retina displays
#endif
#if defined(__TINYC__)
    #define _WIN32_WINNT_WINXP      0x0501
#endif

// NOTE: _GLFW_MIR experimental platform not supported at this moment

#include "context.c"
#include "init.c"
#include "input.c"
#include "monitor.c"
#include "vulkan.c"
#include "window.c"

#if (defined _WIN32 | defined _WIN64)
    #include "win32_init.c"
    #include "win32_joystick.c"
    #include "win32_monitor.c"
    #include "win32_time.c"
    #include "win32_thread.c"
    #include "win32_window.c"
    #include "wgl_context.c"
    #include "egl_context.c"
    #include "osmesa_context.c"
#endif

#if (defined __linux__ || defined _linux || defined __linux)
    #if defined(_GLFW_WAYLAND)
        #include "wl_init.c"
        #include "wl_monitor.c"
        #include "wl_window.c"
        #include "wayland-pointer-constraints-unstable-v1-client-protocol.c"
        #include "wayland-relative-pointer-unstable-v1-client-protocol.c"
        #endif
    #if defined(_GLFW_X11)
        #include "x11_init.c"
        #include "x11_monitor.c"
        #include "x11_window.c"
        #include "glx_context.c"
    #endif

    #include "linux_joystick.c"
    #include "posix_thread.c"
    #include "posix_time.c"
    #include "xkb_unicode.c"
    #include "egl_context.c"
    #include "osmesa_context.c"
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
    #include "x11_init.c"
    #include "x11_monitor.c"
    #include "x11_window.c"
    #include "xkb_unicode.c"
    // TODO: Joystick implementation
    #include "null_joystick.c"
    #include "posix_time.c"
    #include "posix_thread.c"
    #include "glx_context.c"
    #include "egl_context.c"
    #include "osmesa_context.c"
#endif

#if defined(__APPLE__)
    #include "cocoa_init.m"
    #include "cocoa_joystick.m"
    #include "cocoa_monitor.m"
    #include "cocoa_window.m"
    #include "cocoa_time.c"
    #include "posix_thread.c"
    #include "nsgl_context.m"
    #include "egl_context.c"
    #include "osmesa_context.c"
#endif

#endif // GLFW_IMPL

#endif // __GLFW_IMPL_H__
