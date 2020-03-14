#include "platform/gs_platform.h"
#include "base/gs_engine.h"

// Hope we got this... Stupid dependencies...
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

/*============================
// Platform Initialization
============================*/

#define __window_from_handle( platform, handle )\
	( (SDL_Window*)( gs_slot_array_get( ( platform )->windows, ( handle ) ) ) )

gs_result sdl_platform_init( struct gs_platform_i* platform  )
{
	gs_println( "Initializing SDL" );

	// Verify platform is valid
	gs_assert( platform );

    SDL_SetHint( SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1" );

	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) 
	{
		gs_println( "SDL_Init Error: %s", SDL_GetError() );
		return gs_result_failure;
	}

	switch ( platform->settings.video.driver )	
	{
		case gs_platform_video_driver_type_opengl: 
		{
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG );
		    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
		    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

			SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
			 
			SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
			SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 32 );

			if ( platform->settings.video.vsync_enabled ) {
				SDL_GL_SetSwapInterval( 1 );
			}

		} break;

		default:
		{
			// Default to no output at all.
			gs_println( "Video format not supported." );
		} break;
	}

    // Construct cursors
    platform->cursors[ gs_platform_cursor_arrow ] 		= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_ARROW );
    platform->cursors[ gs_platform_cursor_ibeam ] 		= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_IBEAM );
    platform->cursors[ gs_platform_cursor_size_nw_se ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENWSE );
    platform->cursors[ gs_platform_cursor_size_ne_sw ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENESW );
    platform->cursors[ gs_platform_cursor_size_ns ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENS );
    platform->cursors[ gs_platform_cursor_size_we ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZEWE );
    platform->cursors[ gs_platform_cursor_size_all ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZEALL );
    platform->cursors[ gs_platform_cursor_hand ] 		= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_HAND );
    platform->cursors[ gs_platform_cursor_no ] 			= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_NO );

	return gs_result_success;
}

gs_result sdl_platform_shutdown( struct gs_platform_i* platform )
{
	// TODO(John): Actually free up memory and shut down resources...
	return gs_result_success;
}

/*============================
// Platform Util
============================*/

u32 sdl_platform_ticks() 
{
	return SDL_GetTicks();
}

void sdl_platform_sleep( u32 ticks )
{
	SDL_Delay( ticks );	
}

f64 sdl_platform_time()
{
	return (f64)SDL_GetTicks();
}

/*============================
// Platform Input
============================*/

gs_platform_keycode __sdl_key_to_gs_keycode( SDL_Keycode code )
{
	switch ( code )
	{
		case SDLK_a: 			return gs_keycode_a; break;
		case SDLK_b: 			return gs_keycode_b; break;
		case SDLK_c: 			return gs_keycode_c; break;
		case SDLK_d: 			return gs_keycode_d; break;
		case SDLK_e: 			return gs_keycode_e; break;
		case SDLK_f: 			return gs_keycode_f; break;
		case SDLK_g: 			return gs_keycode_g; break;
		case SDLK_h: 			return gs_keycode_h; break;
		case SDLK_i: 			return gs_keycode_i; break;
		case SDLK_j: 			return gs_keycode_j; break;
		case SDLK_k: 			return gs_keycode_k; break;
		case SDLK_l: 			return gs_keycode_l; break;
		case SDLK_m: 			return gs_keycode_m; break;
		case SDLK_n: 			return gs_keycode_n; break;
		case SDLK_o: 			return gs_keycode_o; break;
		case SDLK_p: 			return gs_keycode_p; break;
		case SDLK_q: 			return gs_keycode_q; break;
		case SDLK_r: 			return gs_keycode_r; break;
		case SDLK_s: 			return gs_keycode_s; break;
		case SDLK_t: 			return gs_keycode_t; break;
		case SDLK_u: 			return gs_keycode_u; break;
		case SDLK_v: 			return gs_keycode_v; break;
		case SDLK_w: 			return gs_keycode_w; break;
		case SDLK_x: 			return gs_keycode_x; break;
		case SDLK_y: 			return gs_keycode_y; break;
		case SDLK_z: 			return gs_keycode_z; break;
		case SDLK_LSHIFT: 		return gs_keycode_lshift; break;
		case SDLK_RSHIFT:		return gs_keycode_rshift; break;
		case SDLK_LALT: 		return gs_keycode_lalt; break;
		case SDLK_RALT: 		return gs_keycode_ralt; break;
		case SDLK_LCTRL: 		return gs_keycode_lctrl; break;
		case SDLK_RCTRL: 		return gs_keycode_rctrl; break;
		case SDLK_BACKSPACE: 	return gs_keycode_bspace; break;
		case SDLK_BACKSLASH: 	return gs_keycode_bslash; break;
		case SDLK_QUESTION: 	return gs_keycode_qmark; break;
		case SDLK_BACKQUOTE: 	return gs_keycode_tilde; break;
		case SDLK_COMMA: 		return gs_keycode_comma; break;
		case SDLK_PERIOD: 		return gs_keycode_period; break;
		case SDLK_ESCAPE: 		return gs_keycode_esc; break; 
		case SDLK_SPACE: 		return gs_keycode_space; break;
		case SDLK_LEFT: 		return gs_keycode_left; break;
		case SDLK_UP: 			return gs_keycode_up; break;
		case SDLK_RIGHT: 		return gs_keycode_right; break;
		case SDLK_DOWN: 		return gs_keycode_down; break;
		case SDLK_0:			return gs_keycode_zero; break;
		case SDLK_1: 			return gs_keycode_one; break;
		case SDLK_2: 			return gs_keycode_two; break;
		case SDLK_3: 			return gs_keycode_three; break;
		case SDLK_4: 			return gs_keycode_four; break;
		case SDLK_5: 			return gs_keycode_five; break;
		case SDLK_6: 			return gs_keycode_six; break;
		case SDLK_7: 			return gs_keycode_seven; break;
		case SDLK_8: 			return gs_keycode_eight; break;
		case SDLK_9: 			return gs_keycode_nine; break;
		case SDLK_KP_0: 		return gs_keycode_npzero; break;
		case SDLK_KP_1: 		return gs_keycode_npone; break;
		case SDLK_KP_2: 		return gs_keycode_nptwo; break;
		case SDLK_KP_3: 		return gs_keycode_npthree; break;
		case SDLK_KP_4: 		return gs_keycode_npfour; break;
		case SDLK_KP_5: 		return gs_keycode_npfive; break;
		case SDLK_KP_6: 		return gs_keycode_npsix; break;
		case SDLK_KP_7: 		return gs_keycode_npseven; break;
		case SDLK_KP_8: 		return gs_keycode_npeight; break;
		case SDLK_KP_9: 		return gs_keycode_npnine; break;
		case SDLK_CAPSLOCK: 	return gs_keycode_caps; break;
		case SDLK_DELETE: 		return gs_keycode_delete; break;
		case SDLK_END: 			return gs_keycode_end; break;
		case SDLK_F1: 			return gs_keycode_f1; break;
		case SDLK_F2: 			return gs_keycode_f2; break;
		case SDLK_F3: 			return gs_keycode_f3; break;
		case SDLK_F4: 			return gs_keycode_f4; break;
		case SDLK_F5: 			return gs_keycode_f5; break;
		case SDLK_F6: 			return gs_keycode_f6; break;
		case SDLK_F7: 			return gs_keycode_f7; break;
		case SDLK_F8: 			return gs_keycode_f8; break;
		case SDLK_F9: 			return gs_keycode_f9; break;
		case SDLK_F10: 			return gs_keycode_f10; break;
		case SDLK_F11: 			return gs_keycode_f11; break;
		case SDLK_F12: 			return gs_keycode_f12; break;
		case SDLK_HOME: 		return gs_keycode_home; break;
		case SDLK_PLUS: 		return gs_keycode_plus; break;
		case SDLK_MINUS: 		return gs_keycode_minus; break;
		case SDLK_LEFTBRACKET: 	return gs_keycode_lbracket; break;
		case SDLK_RIGHTBRACKET: return gs_keycode_rbracket; break;
		case SDLK_SEMICOLON: 	return gs_keycode_semi_colon; break;
		case SDLK_RETURN: 		return gs_keycode_enter; break;
		case SDLK_INSERT: 		return gs_keycode_insert; break;
		case SDLK_PAGEUP: 		return gs_keycode_pgup; break;
		case SDLK_PAGEDOWN: 	return gs_keycode_pgdown; break;
		case SDLK_NUMLOCKCLEAR: return gs_keycode_numlock; break;
		case SDLK_TAB: 			return gs_keycode_tab; break;
		case SDLK_KP_MULTIPLY: 	return gs_keycode_npmult; break;
		case SDLK_KP_DIVIDE: 	return gs_keycode_npdiv; break;
		case SDLK_KP_PLUS: 		return gs_keycode_npplus; break;
		case SDLK_KP_MINUS: 	return gs_keycode_npminus; break;
		case SDLK_KP_ENTER: 	return gs_keycode_npenter; break;
		case SDLK_KP_DECIMAL: 	return gs_keycode_npdel; break;
		case SDLK_MUTE: 		return gs_keycode_mute; break;
		case SDLK_VOLUMEUP: 	return gs_keycode_volup; break;
		case SDLK_VOLUMEDOWN: 	return gs_keycode_voldown; break;
		case SDLK_PAUSE: 		return gs_keycode_pause; break;
		case SDLK_PRINTSCREEN:  return gs_keycode_print; break;
	}

	// Shouldn't reach here
	return gs_keycode_count;
}

gs_platform_mouse_button_code __sdl_button_to_gs_mouse_button( SDL_Keycode code )
{
	switch ( code )
	{
		case SDL_BUTTON_LEFT: 	return gs_mouse_lbutton; break;
		case SDL_BUTTON_RIGHT: 	return gs_mouse_rbutton; break;
		case SDL_BUTTON_MIDDLE: return gs_mouse_mbutton; break;
	}	

	// Shouldn't reach here
	return gs_mouse_button_code_count;
}

gs_result sdl_process_input( struct gs_platform_input* input )
{
	// Grab platform instance from engine
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_assert( platform );

	gs_assert( input );

	SDL_Event event;

	// Poll for events
	while ( SDL_PollEvent( &event ) )
	{ 
		switch ( event.type )
		{
			case SDL_QUIT:
			{
				return gs_result_success;
			} break;

			case SDL_KEYDOWN:
			{
				platform->press_key( __sdl_key_to_gs_keycode( event.key.keysym.sym ) );
			} break;

			case SDL_KEYUP:
			{
				platform->release_key( __sdl_key_to_gs_keycode( event.key.keysym.sym ) );
			} break;

			case SDL_MOUSEBUTTONDOWN:
			{
				platform->press_mouse_button( __sdl_button_to_gs_mouse_button( event.button.button ) );
			} break;

			case SDL_MOUSEBUTTONUP:
			{
				platform->release_mouse_button( __sdl_button_to_gs_mouse_button( event.button.button ) );
			} break;

			case SDL_MOUSEMOTION:
			{
				input->mouse.position = ( gs_vec2 ){ .x = ( f32 )event.motion.x, .y = ( f32 )event.motion.y };
			} break;

			case SDL_MOUSEWHEEL:
			{
				input->mouse.wheel = ( gs_vec2 ){ .x = event.wheel.x, .y = event.wheel.y };
			} break;

			case SDL_WINDOWEVENT:
			{
				switch( event.window.event )
				{
					case SDL_WINDOWEVENT_LEAVE:
					{
						// Invalidate mouse position if we leave the window
						input->mouse.position = ( gs_vec2 ){ .x = -1.0f, .y = -1.0f };
					} break;	
				}
			} break;
		}
	}

	return gs_result_in_progress;
}


/*============================
// Platform Window
============================*/

gs_result __sdl_init_gl_context( struct gs_platform_i* platform, SDL_Window* win )
{
	// Construct gl context and store in platform layer
    platform->settings.video.graphics.opengl.ctx = SDL_GL_CreateContext( win );

    // Verify that context is valid
	if ( platform->settings.video.graphics.opengl.ctx == NULL ) 
	{
		gs_println( "SDL_CreateWindow Error: %s", SDL_GetError() );
		gs_assert( false );
		return gs_result_failure;
	} 

	//Set up glew (optional but recommended)
	GLenum error = glewInit();
	if ( error != GLEW_OK ) 
	{
		gs_println( "SDL_CreateWindow Error: %s", SDL_GetError() );
		gs_assert( false );
		return gs_result_failure;
	}

	//Set glewExperimental to true
	glewExperimental = GL_TRUE;

	// Set major gl version for platform
	SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, (s32*)&platform->settings.video.graphics.opengl.major_version );

	// Set minor gl version for platform
	SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, (s32*)&platform->settings.video.graphics.opengl.minor_version );

	return gs_result_success;
}

void* sdl_create_window( const char* title, u32 width, u32 height )
{
	// Grab instance of platform API
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	// Assert that platform is registered
	gs_assert( platform );

	switch ( platform->settings.video.driver )
	{
		case gs_platform_video_driver_type_opengl:
		{
			// Want to have window flags provided for the window
			SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_SHOWN |SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
			// Construct new window
			SDL_Window* win = SDL_CreateWindow
			(
				title, 
				SDL_WINDOWPOS_CENTERED, 
				SDL_WINDOWPOS_CENTERED, 
				width, 
				height, 
				window_flags
			);

			if ( win == NULL ) 
			{
				gs_println( "SDL_CreateWindow Error: %s", SDL_GetError() );
				gs_assert( false );
				return NULL;
			}

			// For now, the the window will be an opengl window if SDL is used.
			// Single, static opengl context
			if ( !platform->settings.video.graphics.opengl.ctx )
			{
				gs_result res = __sdl_init_gl_context( platform, win );
				if ( res != gs_result_success )
				{
					gs_println( "SDL_CreateWindow Error: %s", SDL_GetError() );
					gs_assert( false );
					return NULL;
				}
			}

		    SDL_GL_MakeCurrent( win, platform->settings.video.graphics.opengl.ctx );

		    return (void*)win;

		} break;

		default: 
		{
			gs_println( "Error: graphics driver type not supported." );
			gs_assert( false );
		} break;

		return NULL;
	}

	// Shouldn't get here
	return NULL;
}

void sdl_window_swap_buffer( gs_platform_window_handle handle )
{
	// Grab window from plaform layer slot array
	SDL_Window* win = __window_from_handle( gs_engine_instance()->ctx.platform, handle );
    SDL_GL_SwapWindow( win );
}

gs_vec2 sdl_window_size( gs_platform_window_handle handle )
{
	SDL_Window* win = __window_from_handle( gs_engine_instance()->ctx.platform, handle );
	s32 w, h;
	SDL_GetWindowSize( win, &w, &h );	
	return ( gs_vec2 ) { .x = w, .y = h };
}

void sdl_window_size_w_h( gs_platform_window_handle handle, s32* w, s32* h )
{
	SDL_Window* win = __window_from_handle( gs_engine_instance()->ctx.platform, handle );
	SDL_GetWindowSize( win, w, h );
}

void sdl_set_window_size( gs_platform_window_handle handle, s32 w, s32 h )
{
	SDL_Window* win = __window_from_handle( gs_engine_instance()->ctx.platform, handle );
	SDL_SetWindowSize( win, w, h );
}

void sdl_set_cursor( gs_platform_window_handle handle, gs_platform_cursor cursor )
{
	SDL_Cursor* cp = ( SDL_Cursor* )gs_engine_instance()->ctx.platform->cursors[ cursor ];
	SDL_SetCursor( cp );
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
	platform->init 		= &sdl_platform_init;
	platform->shutdown 	= &sdl_platform_shutdown;

	/*============================
	// Platform Util
	============================*/
	platform->ticks		= &sdl_platform_ticks;
	platform->sleep		= &sdl_platform_sleep;
	platform->get_time 	= &sdl_platform_time;

	/*============================
	// Platform Input
	============================*/
	platform->process_input 	= &sdl_process_input;

	/*============================
	// Platform Window
	============================*/
	platform->create_window_internal 	= &sdl_create_window;
	platform->window_swap_buffer 		= &sdl_window_swap_buffer;
	platform->window_size 				= &sdl_window_size;
	platform->window_size_w_h 			= &sdl_window_size_w_h;
	platform->set_window_size 			= &sdl_set_window_size;
	platform->set_cursor 				= &sdl_set_cursor;

	// Todo(John): Remove this from the default initialization and make it a part of a plugin or config setting
	platform->settings.video.driver = gs_platform_video_driver_type_opengl;

	return platform;
}




