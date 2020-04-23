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

#include "third_party/source/GLFW/context.c"
#include "third_party/source/GLFW/init.c"
#include "third_party/source/GLFW/input.c"
#include "third_party/source/GLFW/monitor.c"
#include "third_party/source/GLFW/vulkan.c"
#include "third_party/source/GLFW/window.c"

#if ( defined _WIN32 | defined _WIN64 )
    #include "third_party/source/GLFW/win32_init.c"
    #include "third_party/source/GLFW/win32_joystick.c"
    #include "third_party/source/GLFW/win32_monitor.c"
    #include "third_party/source/GLFW/win32_time.c"
    #include "third_party/source/GLFW/win32_thread.c"
    #include "third_party/source/GLFW/win32_window.c"
    #include "third_party/source/GLFW/wgl_context.c"
    #include "third_party/source/GLFW/egl_context.c"
    #include "third_party/source/GLFW/osmesa_context.c"
#endif

#if ( defined __linux__ || defined _linux || defined __linux )
    #if defined(_GLFW_WAYLAND)
        #include "third_party/source/GLFW/wl_init.c"
        #include "third_party/source/GLFW/wl_monitor.c"
        #include "third_party/source/GLFW/wl_window.c"
        #include "third_party/source/GLFW/wayland-pointer-constraints-unstable-v1-client-protocol.c"
        #include "third_party/source/GLFW/wayland-relative-pointer-unstable-v1-client-protocol.c"
        #endif
    #if defined(_GLFW_X11)
        #include "third_party/source/GLFW/x11_init.c"
        #include "third_party/source/GLFW/x11_monitor.c"
        #include "third_party/source/GLFW/x11_window.c"
        #include "third_party/source/GLFW/glx_context.c"
    #endif

    #include "third_party/source/GLFW/linux_joystick.c"
    #include "third_party/source/GLFW/posix_thread.c"
    #include "third_party/source/GLFW/posix_time.c"
    #include "third_party/source/GLFW/xkb_unicode.c"
    #include "third_party/source/GLFW/egl_context.c"
    #include "third_party/source/GLFW/osmesa_context.c"
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined( __NetBSD__) || defined(__DragonFly__)
    #include "third_party/source/GLFW/x11_init.c"
    #include "third_party/source/GLFW/x11_monitor.c"
    #include "third_party/source/GLFW/x11_window.c"
    #include "third_party/source/GLFW/xkb_unicode.c"
    // TODO: Joystick implementation
    #include "third_party/source/GLFW/null_joystick.c"
    #include "third_party/source/GLFW/posix_time.c"
    #include "third_party/source/GLFW/posix_thread.c"
    #include "third_party/source/GLFW/glx_context.c"
    #include "third_party/source/GLFW/egl_context.c"
    #include "third_party/source/GLFW/osmesa_context.c"
#endif

#if defined(__APPLE__)
    #include "third_party/source/GLFW/cocoa_init.m"
    #include "third_party/source/GLFW/cocoa_joystick.m"
    #include "third_party/source/GLFW/cocoa_monitor.m"
    #include "third_party/source/GLFW/cocoa_window.m"
    #include "third_party/source/GLFW/cocoa_time.c"
    #include "third_party/source/GLFW/posix_thread.c"
    #include "third_party/source/GLFW/nsgl_context.m"
    #include "third_party/source/GLFW/egl_context.c"
    #include "third_party/source/GLFW/osmesa_context.c"
#endif
