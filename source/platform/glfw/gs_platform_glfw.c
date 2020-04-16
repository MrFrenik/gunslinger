#include "base/gs_engine.h"
#include "platform/gs_platform.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

// Forward Decls.
void __glfw_key_callback( GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods );
void __glfw_mouse_button_callback( GLFWwindow* window, s32 button, s32 action, s32 mods );
void __glfw_mouse_cursor_position_callback( GLFWwindow* window, f64 x, f64 y );
void __glfw_mouse_scroll_wheel_callback( GLFWwindow* window, f64 xoffset, f64 yoffset );
void __glfw_mouse_cursor_enter_callback( GLFWwindow* window, s32 entered );
void __glfw_frame_buffer_size_callback( GLFWwindow* window, s32 width, s32 height );
void __glfw_drop_callback( GLFWwindow* window );

#define __window_from_handle( platform, handle )\
	( (GLFWwindow*)( gs_slot_array_get( ( platform )->windows, ( handle ) ) ) )

/*============================
// Platform Initialization
============================*/

gs_result glfw_platform_init( struct gs_platform_i* platform  )
{
	gs_println( "Initializing GLFW" );

	glfwInit();

	// Verify platform is valid
	gs_assert( platform );

	switch ( platform->settings.video.driver )	
	{
		case gs_platform_video_driver_type_opengl: 
		{
			#if ( defined  GS_PLATFORM_APPLE )
				glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
				glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
				glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
				glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
			#else
				// glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, platform->settings.video.graphics.opengl.major_version );
				// glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, platform->settings.video.graphics.opengl.minor_version );
				glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
				glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
				glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
				glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
			#endif

			// glfwSwapInterval( platform->settings.video.vsync_enabled );
			// glfwSwapInterval( 0 );

		} break;

		default:
		{
			// Default to no output at all.
			gs_println( "Video format not supported." );
		} break;
	}

    // Construct cursors
    platform->cursors[ (u32)gs_platform_cursor_arrow ] 		= glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
    platform->cursors[ (u32)gs_platform_cursor_ibeam ] 		= glfwCreateStandardCursor( GLFW_IBEAM_CURSOR );
    platform->cursors[ (u32)gs_platform_cursor_size_nw_se ] = glfwCreateStandardCursor( GLFW_CROSSHAIR_CURSOR );
    platform->cursors[ (u32)gs_platform_cursor_size_ne_sw ] = glfwCreateStandardCursor( GLFW_CROSSHAIR_CURSOR );
    platform->cursors[ (u32)gs_platform_cursor_size_ns ] 	= glfwCreateStandardCursor( GLFW_VRESIZE_CURSOR );
    platform->cursors[ (u32)gs_platform_cursor_size_we ] 	= glfwCreateStandardCursor( GLFW_HRESIZE_CURSOR );
    platform->cursors[ (u32)gs_platform_cursor_size_all ] 	= glfwCreateStandardCursor( GLFW_CROSSHAIR_CURSOR );
    platform->cursors[ (u32)gs_platform_cursor_hand ] 		= glfwCreateStandardCursor( GLFW_HAND_CURSOR );
    platform->cursors[ (u32)gs_platform_cursor_no ] 		= glfwCreateStandardCursor( GLFW_ARROW_CURSOR );

	return gs_result_success;
}

gs_result glfw_platform_shutdown( struct gs_platform_i* platform )
{
	return gs_result_success;
}

/*============================
// Platform Util
============================*/

// Returns in milliseconds
f64 glfw_platform_time()
{
	return (glfwGetTime() * 1000.0);
}

#if ( defined GS_PLATFORM_APPLE || defined GS_PLATFORM_LINUX )

	#include <sched.h>
	#include <unistd.h>

#elif ( defined GS_PLATFORM_WINDOWS )

	#include <windows.h>
#endif

void glfw_platform_sleep( f32 ms ) 
{
	#if ( defined GS_PLATFORM_WIN )

			_sleep( ms );

	#elif ( defined GS_PLATFORM_APPLE )

	        usleep( ms * 1000.f ); // unistd.h

	#endif
}

gs_platform_keycode glfw_key_to_gs_keycode( u32 code )
{
	switch ( code )
	{
		case GLFW_KEY_A: 			return gs_keycode_a; break;
		case GLFW_KEY_B: 			return gs_keycode_b; break;
		case GLFW_KEY_C: 			return gs_keycode_c; break;
		case GLFW_KEY_D: 			return gs_keycode_d; break;
		case GLFW_KEY_E: 			return gs_keycode_e; break;
		case GLFW_KEY_F: 			return gs_keycode_f; break;
		case GLFW_KEY_G: 			return gs_keycode_g; break;
		case GLFW_KEY_H: 			return gs_keycode_h; break;
		case GLFW_KEY_I: 			return gs_keycode_i; break;
		case GLFW_KEY_J: 			return gs_keycode_j; break;
		case GLFW_KEY_K: 			return gs_keycode_k; break;
		case GLFW_KEY_L: 			return gs_keycode_l; break;
		case GLFW_KEY_M: 			return gs_keycode_m; break;
		case GLFW_KEY_N: 			return gs_keycode_n; break;
		case GLFW_KEY_O: 			return gs_keycode_o; break;
		case GLFW_KEY_P: 			return gs_keycode_p; break;
		case GLFW_KEY_Q: 			return gs_keycode_q; break;
		case GLFW_KEY_R: 			return gs_keycode_r; break;
		case GLFW_KEY_S: 			return gs_keycode_s; break;
		case GLFW_KEY_T: 			return gs_keycode_t; break;
		case GLFW_KEY_U: 			return gs_keycode_u; break;
		case GLFW_KEY_V: 			return gs_keycode_v; break;
		case GLFW_KEY_W: 			return gs_keycode_w; break;
		case GLFW_KEY_X: 			return gs_keycode_x; break;
		case GLFW_KEY_Y: 			return gs_keycode_y; break;
		case GLFW_KEY_Z: 			return gs_keycode_z; break;
		case GLFW_KEY_LEFT_SHIFT: 		return gs_keycode_lshift; break;
		case GLFW_KEY_RIGHT_SHIFT:		return gs_keycode_rshift; break;
		case GLFW_KEY_LEFT_ALT: 		return gs_keycode_lalt; break;
		case GLFW_KEY_RIGHT_ALT: 		return gs_keycode_ralt; break;
		case GLFW_KEY_LEFT_CONTROL: 		return gs_keycode_lctrl; break;
		case GLFW_KEY_RIGHT_CONTROL: 		return gs_keycode_rctrl; break;
		case GLFW_KEY_BACKSPACE: 	return gs_keycode_bspace; break;
		case GLFW_KEY_BACKSLASH: 	return gs_keycode_bslash; break;
		case GLFW_KEY_SLASH: 	return gs_keycode_qmark; break;
		case GLFW_KEY_GRAVE_ACCENT: 	return gs_keycode_tilde; break;
		case GLFW_KEY_COMMA: 		return gs_keycode_comma; break;
		case GLFW_KEY_PERIOD: 		return gs_keycode_period; break;
		case GLFW_KEY_ESCAPE: 		return gs_keycode_esc; break; 
		case GLFW_KEY_SPACE: 		return gs_keycode_space; break;
		case GLFW_KEY_LEFT: 		return gs_keycode_left; break;
		case GLFW_KEY_UP: 			return gs_keycode_up; break;
		case GLFW_KEY_RIGHT: 		return gs_keycode_right; break;
		case GLFW_KEY_DOWN: 	return gs_keycode_down; break;
		case GLFW_KEY_0:		return gs_keycode_zero; break;
		case GLFW_KEY_1: 			return gs_keycode_one; break;
		case GLFW_KEY_2: 			return gs_keycode_two; break;
		case GLFW_KEY_3: 			return gs_keycode_three; break;
		case GLFW_KEY_4: 			return gs_keycode_four; break;
		case GLFW_KEY_5: 			return gs_keycode_five; break;
		case GLFW_KEY_6: 			return gs_keycode_six; break;
		case GLFW_KEY_7: 			return gs_keycode_seven; break;
		case GLFW_KEY_8: 			return gs_keycode_eight; break;
		case GLFW_KEY_9: 			return gs_keycode_nine; break;
		case GLFW_KEY_KP_0: 		return gs_keycode_npzero; break;
		case GLFW_KEY_KP_1: 		return gs_keycode_npone; break;
		case GLFW_KEY_KP_2: 		return gs_keycode_nptwo; break;
		case GLFW_KEY_KP_3: 		return gs_keycode_npthree; break;
		case GLFW_KEY_KP_4: 		return gs_keycode_npfour; break;
		case GLFW_KEY_KP_5: 		return gs_keycode_npfive; break;
		case GLFW_KEY_KP_6: 		return gs_keycode_npsix; break;
		case GLFW_KEY_KP_7: 		return gs_keycode_npseven; break;
		case GLFW_KEY_KP_8: 		return gs_keycode_npeight; break;
		case GLFW_KEY_KP_9: 		return gs_keycode_npnine; break;
		case GLFW_KEY_CAPS_LOCK: 	return gs_keycode_caps; break;
		case GLFW_KEY_DELETE: 		return gs_keycode_delete; break;
		case GLFW_KEY_END: 			return gs_keycode_end; break;
		case GLFW_KEY_F1: 			return gs_keycode_f1; break;
		case GLFW_KEY_F2: 			return gs_keycode_f2; break;
		case GLFW_KEY_F3: 			return gs_keycode_f3; break;
		case GLFW_KEY_F4: 			return gs_keycode_f4; break;
		case GLFW_KEY_F5: 			return gs_keycode_f5; break;
		case GLFW_KEY_F6: 			return gs_keycode_f6; break;
		case GLFW_KEY_F7: 			return gs_keycode_f7; break;
		case GLFW_KEY_F8: 			return gs_keycode_f8; break;
		case GLFW_KEY_F9: 			return gs_keycode_f9; break;
		case GLFW_KEY_F10: 			return gs_keycode_f10; break;
		case GLFW_KEY_F11: 			return gs_keycode_f11; break;
		case GLFW_KEY_F12: 			return gs_keycode_f12; break;
		case GLFW_KEY_HOME: 		return gs_keycode_home; break;
		case GLFW_KEY_EQUAL: 		return gs_keycode_plus; break;
		case GLFW_KEY_MINUS: 		return gs_keycode_minus; break;
		case GLFW_KEY_LEFT_BRACKET: 	return gs_keycode_lbracket; break;
		case GLFW_KEY_RIGHT_BRACKET: return gs_keycode_rbracket; break;
		case GLFW_KEY_SEMICOLON: 	return gs_keycode_semi_colon; break;
		case GLFW_KEY_ENTER: 		return gs_keycode_enter; break;
		case GLFW_KEY_INSERT: 		return gs_keycode_insert; break;
		case GLFW_KEY_PAGE_UP: 		return gs_keycode_pgup; break;
		case GLFW_KEY_PAGE_DOWN: 	return gs_keycode_pgdown; break;
		case GLFW_KEY_NUM_LOCK: return gs_keycode_numlock; break;
		case GLFW_KEY_TAB: 			return gs_keycode_tab; break;
		case GLFW_KEY_KP_MULTIPLY: 	return gs_keycode_npmult; break;
		case GLFW_KEY_KP_DIVIDE: 	return gs_keycode_npdiv; break;
		case GLFW_KEY_KP_ADD: 		return gs_keycode_npplus; break;
		case GLFW_KEY_KP_SUBTRACT: 	return gs_keycode_npminus; break;
		case GLFW_KEY_KP_ENTER: 	return gs_keycode_npenter; break;
		case GLFW_KEY_KP_DECIMAL: 	return gs_keycode_npdel; break;
		case GLFW_KEY_PAUSE: 		return gs_keycode_pause; break;
		case GLFW_KEY_PRINT_SCREEN:  return gs_keycode_print; break;
		case GLFW_KEY_UNKNOWN: 		return gs_keycode_count; break;
	}

	// Shouldn't reach here
	return gs_keycode_count;
}

gs_platform_mouse_button_code __glfw_button_to_gs_mouse_button( s32 code )
{
	switch ( code )
	{
		case GLFW_MOUSE_BUTTON_LEFT: 	return gs_mouse_lbutton; break;
		case GLFW_MOUSE_BUTTON_RIGHT: 	return gs_mouse_rbutton; break;
		case GLFW_MOUSE_BUTTON_MIDDLE: return gs_mouse_mbutton; break;
	}	

	// Shouldn't reach here
	return gs_mouse_button_code_count;
}

void __glfw_key_callback( GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods ) 
{
	// Grab platform instance from engine
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Get keycode from key
	gs_platform_keycode code = glfw_key_to_gs_keycode( key );

	switch ( action )
	{
		// Released
		case 0: {
			platform->release_key( code );
		} break;

		// Pressed
		case 1: {
			platform->press_key( code );
		} break;

		default: {
		} break;
	}
}

void __glfw_mouse_button_callback( GLFWwindow* window, s32 button, s32 action, s32 mods )
{
	// Grab platform instance from engine
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Get mouse code from key
	gs_platform_mouse_button_code code = __glfw_button_to_gs_mouse_button( button );

	switch ( action )
	{
		// Released
		case 0:
		{
			platform->release_mouse_button( code );
		} break;

		// Pressed
		case 1:
		{
			platform->press_mouse_button( code );
		} break;
	}
}

void __glfw_mouse_cursor_position_callback( GLFWwindow* window, f64 x, f64 y )
{
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	platform->input.mouse.position = (gs_vec2){ x, y };
}

void __glfw_mouse_scroll_wheel_callback( GLFWwindow* window, f64 x, f64 y )
{
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	platform->input.mouse.wheel = (gs_vec2){ (f32)x, (f32)y };
}

// Gets called when mouse enters or leaves frame of window
void __glfw_mouse_cursor_enter_callback( GLFWwindow* window, s32 entered )
{
	// Nothing for now, will capture state for windows later
}

void __glfw_frame_buffer_size_callback( GLFWwindow* window, s32 width, s32 height )
{
	// gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	// if ( gfx )
	// {
	// 	gfx->set_viewport( width, height );
	// }	
}

gs_result glfw_process_input( struct gs_platform_input* input )
{
	glfwPollEvents();

	return gs_result_in_progress;
}

void* glfw_create_window( const char* title, u32 width, u32 height )
{
    // GLFWwindow* window = glfwCreateWindow(width, height, title, glfwGetPrimaryMonitor(), NULL);
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        // std::cout << "Failed to create GLFW window" << std::endl;
        gs_println( "Failed to create window." );
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
	glfwSetKeyCallback( window, &__glfw_key_callback );
	glfwSetMouseButtonCallback( window, &__glfw_mouse_button_callback );
	glfwSetCursorPosCallback( window, &__glfw_mouse_cursor_position_callback );
	glfwSetScrollCallback( window, &__glfw_mouse_scroll_wheel_callback );
	glfwSetCursorEnterCallback( window, &__glfw_mouse_cursor_enter_callback );
	glfwSetFramebufferSizeCallback(window, &__glfw_frame_buffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		gs_println( "Failed to initialize GLFW." );
		return NULL;
	}

	return window;
}

void glfw_window_swap_buffer( gs_resource_handle handle )
{
	// Grab instance of platform from engine
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_assert( platform );

	// Grab window from handle
	GLFWwindow* win = __window_from_handle( gs_engine_instance()->ctx.platform, handle );
	gs_assert( win );

    glfwSwapBuffers( win );
}

void glfw_set_window_size( gs_resource_handle handle, s32 w, s32 h )
{
	GLFWwindow* win = __window_from_handle( gs_engine_instance()->ctx.platform, handle );
	gs_assert( win );
	glfwSetWindowSize( win, w, h );
}

gs_vec2 glfw_window_size( gs_resource_handle handle )
{
	GLFWwindow* win = __window_from_handle( gs_engine_instance()->ctx.platform, handle );
	gs_assert( win );
	s32 w, h;
	glfwGetWindowSize( win, &w, &h );
	return ( gs_vec2 ) { .x = w, .y = h };
}

void glfw_window_size_w_h( gs_resource_handle handle, s32* w, s32* h )
{
	GLFWwindow* win = __window_from_handle( gs_engine_instance()->ctx.platform, handle );
	gs_assert( win );
	glfwGetWindowSize( win, w, h );
}

void glfw_set_cursor( gs_resource_handle handle, gs_platform_cursor cursor )
{
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	GLFWwindow* win = __window_from_handle( platform, handle );
	GLFWcursor* cp = ((GLFWcursor*)platform->cursors[ (u32)cursor ]); 
	glfwSetCursor( win, cp );
}

void glfw_set_vsync_enabled( b32 enabled )
{
	glfwSwapInterval( enabled ? 1 : 0 );
}

void glfw_set_dropped_files_callback( gs_resource_handle handle, dropped_files_callback_t callback )
{
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	GLFWwindow* win = __window_from_handle( platform, handle );
	glfwSetDropCallback( win, (GLFWdropfun)callback );
}

void  glfw_set_window_close_callback( gs_resource_handle handle, window_close_callback_t callback )
{
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	GLFWwindow* win = __window_from_handle( platform, handle );
	glfwSetWindowCloseCallback( win, (GLFWwindowclosefun)callback );
}

// Method for creating platform layer for SDL
struct gs_platform_i* gs_platform_construct()
{
	// Construct new platform interface
	struct gs_platform_i* platform = gs_malloc_init( gs_platform_i );

	/*
	 	Initialize platform interface with all appropriate function pointers
	*/

	/*============================
	// Platform Initialization
	============================*/
	platform->init 		= &glfw_platform_init;
	platform->shutdown 	= &glfw_platform_shutdown;

	/*============================
	// Platform Util
	============================*/
	platform->sleep			= &glfw_platform_sleep;
	platform->elapsed_time 	= &glfw_platform_time;

	/*============================
	// Platform Video
	============================*/
	platform->enable_vsync = &glfw_set_vsync_enabled;

	/*============================
	// Platform Input
	============================*/
	platform->process_input 	= &glfw_process_input;

	/*============================
	// Platform Window
	============================*/
	platform->create_window_internal		= &glfw_create_window;
	platform->window_swap_buffer 			= &glfw_window_swap_buffer;
	platform->window_size 					= &glfw_window_size;
	platform->window_size_w_h 				= &glfw_window_size_w_h;
	platform->set_window_size 				= &glfw_set_window_size;
	platform->set_cursor 					= &glfw_set_cursor;
	platform->set_dropped_files_callback 	= &glfw_set_dropped_files_callback;
	platform->set_window_close_callback 	= &glfw_set_window_close_callback;

	// Todo(John): Remove this from the default initialization and make it a part of a plugin or config setting
	platform->settings.video.driver = gs_platform_video_driver_type_opengl;
	platform->settings.video.graphics.opengl.major_version = 3;
	platform->settings.video.graphics.opengl.minor_version = 3;

	return platform;
}















