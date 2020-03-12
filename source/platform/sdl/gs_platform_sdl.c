#include "platform/gs_platform.h"
#include "base/gs_engine.h"

// Hope we got this... Stupid dependencies...
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Specific implementation for platform window for SDL2
typedef struct gs_platform_window {
	SDL_Window* sdl_window;	
	SDL_Cursor* cursors[ gs_platform_cursor_count ];
} gs_platform_window;

/*============================
// Platform Initialization
============================*/

gs_result sdl_platform_init( struct gs_platform_i* platform  )
{
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

			// Set on vsync by default
			SDL_GL_SetSwapInterval( 1 );
		} break;

		default:
		{
			// Default to no output at all.
			gs_println( "Video format not supported." );
		} break;
	}

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

void sdl_platform_delay( u32 ticks )
{
	SDL_Delay( ticks );	
}

/*============================
// Platform UUID
============================*/

struct gs_uuid sdl_generate_uuid()
{
	struct gs_uuid uuid;

	srand( clock() );
	char guid[40];
	s32 t = 0;
	char* sz_temp = "xxxxxxxxxxxx4xxxyxxxxxxxxxxxxxxx";
	char* sz_hex = "0123456789abcdef-";
	s32 n_len = strlen( sz_temp );

	for ( t=0; t < n_len + 1; t++ )
	{
	    s32 r = rand () % 16;
	    char c = ' ';   

	    switch ( sz_temp[t] )
	    {
	        case 'x' : { c = sz_hex [r]; } break;
	        case 'y' : { c = sz_hex [( r & 0x03 ) | 0x08]; } break;
	        case '-' : { c = '-'; } break;
	        case '4' : { c = '4'; } break;
	    }

	    guid[t] = ( t < n_len ) ? c : 0x00;
	}

	// Convert to uuid bytes from string
	const char* hex_string = sz_temp, *pos = hex_string;

     /* WARNING: no sanitization or error-checking whatsoever */
    for ( usize count = 0; count < 16; count++) 
    {
        sscanf( pos, "%2hhx", &uuid.bytes[count] );
        pos += 2;
    }

	return uuid;
}

// Mutable temp buffer 'tmp_buffer'
void sdl_uuid_to_string( char* tmp_buffer, const struct gs_uuid* uuid )
{
	gs_snprintf 
	( 
		tmp_buffer, 
		32,
		"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		uuid->bytes[ 0 ],
		uuid->bytes[ 1 ],
		uuid->bytes[ 2 ],
		uuid->bytes[ 3 ],
		uuid->bytes[ 4 ],
		uuid->bytes[ 5 ],
		uuid->bytes[ 6 ],
		uuid->bytes[ 7 ],
		uuid->bytes[ 8 ],
		uuid->bytes[ 9 ],
		uuid->bytes[ 10 ],
		uuid->bytes[ 11 ],
		uuid->bytes[ 12 ],
		uuid->bytes[ 13 ],
		uuid->bytes[ 14 ],
		uuid->bytes[ 15 ]
	);
}

u32 sdl_hash_uuid( const struct gs_uuid* uuid )
{
	char temp_buffer[] = gs_uuid_temp_str_buffer();
	sdl_uuid_to_string( temp_buffer, uuid );
	return ( gs_hash_str( temp_buffer ) );
}

/*============================
// Platform Input
============================*/

typedef struct gs_platform_mouse
{
	b32 button_map[ gs_mouse_button_code_count ];
	b32 prev_button_map[ gs_mouse_button_code_count ];
	gs_vec2 position;
	gs_vec2 prev_position;
	gs_vec2 delta;
	gs_vec2 wheel;
} gs_platform_mouse;

typedef struct gs_platform_input
{
	b32 key_map[ gs_keycode_count ];
	b32 prev_key_map[ gs_keycode_count ];
	gs_platform_mouse mouse;
} gs_platform_input;

gs_platform_keycode _sdl_key_to_gs_keycode( SDL_Keycode code )
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

gs_mouse_button_code _sdl_button_to_gs_mouse_button( SDL_Keycode code )
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

struct gs_platform_input* sdl_create_input()
{
	gs_platform_input* input = gs_malloc_init( gs_platform_input );
	return input;
}

b32 sdl_was_key_down( struct gs_platform_input* input, gs_platform_keycode code )
{
	return ( input->prev_key_map[ code ] );
}

b32 sdl_key_down( struct gs_platform_input* input, gs_platform_keycode code )
{
	return ( input->key_map[ code ] );
}

b32 sdl_key_pressed( struct gs_platform_input* input, gs_platform_keycode code )
{
	if ( sdl_key_down( input, code ) && !sdl_was_key_down( input, code ) )
	{
		return true;
	}
	return false;
}

b32 sdl_key_released( struct gs_platform_input* input, gs_platform_keycode code )
{
	return ( sdl_was_key_down( input, code ) && !sdl_key_down( input, code ) );
}

b32 sdl_was_mouse_down( struct gs_platform_input* input, gs_mouse_button_code code )
{
	return ( input->mouse.prev_button_map[ code ] );
}

void sdl_press_mouse_button( struct gs_platform_input* input, gs_mouse_button_code code )
{
	if ( (u32)code < (u32)gs_mouse_button_code_count ) 
	{
		input->mouse.button_map[ code ] = true;
	}
}

void sdl_release_mouse_button( struct gs_platform_input* input, gs_mouse_button_code code )
{
	if ( (u32)code < (u32)gs_mouse_button_code_count ) 
	{
		input->mouse.button_map[ code ] = false;
	}
}

b32 sdl_mouse_down( struct gs_platform_input* input, gs_mouse_button_code code )
{
	return ( input->mouse.button_map[ code ] );
}

b32 sdl_mouse_pressed( struct gs_platform_input* input, gs_mouse_button_code code )
{
	if ( sdl_mouse_down( input, code ) && !sdl_was_mouse_down( input, code ) )
	{
		return true;
	}
	return false;
}

b32 sdl_mouse_released( struct gs_platform_input* input, gs_mouse_button_code code )
{
	return ( sdl_was_mouse_down( input, code ) && !sdl_mouse_down( input, code ) );
}

gs_vec2 sdl_mouse_delta( struct gs_platform_input* input )
{
	if (input->mouse.prev_position.x < 0.0f || 
		input->mouse.prev_position.y < 0.0f ||
		input->mouse.position.x < 0.0f || 
		input->mouse.position.y < 0.0f )
	{
		return (gs_vec2){ 0.0f, 0.0f };
	}
	
	return (gs_vec2){ input->mouse.position.x - input->mouse.prev_position.x, 
					  input->mouse.position.y - input->mouse.prev_position.y };
}

gs_vec2 sdl_mouse_position( struct gs_platform_input* input )
{
	return ( gs_vec2 ) 
	{
		.x = input->mouse.position.x, 
		.y = input->mouse.position.y
	};
}

void sdl_mouse_position_x_y( struct gs_platform_input* input, f32* x, f32* y )
{
	*x = input->mouse.position.x;
	*y = input->mouse.position.y;
}

void sdl_mouse_wheel( struct gs_platform_input* input, f32* x, f32* y )
{
	*x = input->mouse.wheel.x;
	*y = input->mouse.wheel.y;	
}

void sdl_press_key( struct gs_platform_input* input, gs_platform_keycode code )
{
	if ( code < gs_keycode_count ) 
	{
		input->key_map[ code ] = true;
	}
}

void sdl_release_key( struct gs_platform_input* input, gs_platform_keycode code )
{
	if ( code < gs_keycode_count ) 
	{
		input->key_map[ code ] = false;
	}
}

gs_result sdl_process_input( struct gs_platform_input* input )
{
	SDL_Event event;

	gs_assert( input );

	// Update all input and mouse keys from previous frame
	// __gs_platform_input_update( input );

	// Update all input and mouse keys from previous frame
	// Previous key presses
	gs_for_range_i( gs_keycode_count )
	{
		input->prev_key_map[ i ] = input->key_map[ i ];
	}

	// Previous mouse button presses
	gs_for_range_i( gs_mouse_button_code_count )
	{
		input->mouse.prev_button_map[ i ] = input->mouse.button_map[ i ];
	}

	input->mouse.prev_position = input->mouse.position;
	input->mouse.wheel = (gs_vec2){ 0.0f, 0.0f };

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
				sdl_press_key( input, _sdl_key_to_gs_keycode( event.key.keysym.sym ) );
			} break;

			case SDL_KEYUP:
			{
				sdl_release_key( input, _sdl_key_to_gs_keycode( event.key.keysym.sym ) );
			} break;

			case SDL_MOUSEBUTTONDOWN:
			{
				sdl_press_mouse_button( input, _sdl_button_to_gs_mouse_button( event.button.button ) );
			} break;

			case SDL_MOUSEBUTTONUP:
			{
				sdl_release_mouse_button( input, _sdl_button_to_gs_mouse_button( event.button.button ) );
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

struct gs_platform_window* sdl_create_window( const char* title, u32 width, u32 height )
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

			gs_platform_window* gwin = gs_malloc( sizeof( gs_platform_window ) );
			gs_assert( gwin );
			gwin->sdl_window = win;

		    SDL_GL_MakeCurrent( win, platform->settings.video.graphics.opengl.ctx );

		    // Construct cursors
		    // NOTE(john): For now, just put in window, but should be abstracted out into a general context later on
		    gwin->cursors[ gs_platform_cursor_arrow ] 		= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_ARROW );
		    gwin->cursors[ gs_platform_cursor_ibeam ] 		= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_IBEAM );
		    gwin->cursors[ gs_platform_cursor_size_nw_se ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENWSE );
		    gwin->cursors[ gs_platform_cursor_size_ne_sw ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENESW );
		    gwin->cursors[ gs_platform_cursor_size_ns ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZENS );
		    gwin->cursors[ gs_platform_cursor_size_we ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZEWE );
		    gwin->cursors[ gs_platform_cursor_size_all ] 	= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_SIZEALL );
		    gwin->cursors[ gs_platform_cursor_hand ] 		= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_HAND );
		    gwin->cursors[ gs_platform_cursor_no ] 			= SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_NO );

			return ( gs_platform_window* )gwin;

		} break;

		default: 
		{
			gs_println( "Error: graphics driver type not supported." );
			gs_assert( false );
		} break;
	}
}

void sdl_window_swap_buffer( struct gs_platform_window* win )
{
    SDL_GL_SwapWindow( (SDL_Window* )win->sdl_window );
}

gs_vec2 sdl_window_size( struct gs_platform_window* win )
{
	s32 w, h;
	SDL_GetWindowSize( ( SDL_Window* )win->sdl_window, &w, &h );	
	return ( gs_vec2 ) { .x = w, .y = h };
}

void sdl_window_size_w_h( struct gs_platform_window* win, s32* w, s32* h )
{
	SDL_GetWindowSize( ( SDL_Window* )win->sdl_window, w, h );
}

void sdl_set_cursor( struct gs_platform_window* win, gs_platform_cursor cursor )
{
	SDL_SetCursor( win->cursors[ cursor] );
}

/*============================
// Platform File IO
============================*/

char* sdl_read_file_contents_into_string_null_term( const char* file_path, const char* mode, usize* sz )
{
	char* buffer = 0;
	FILE* fp = fopen( file_path, mode );
	if ( fp )
	{
		fseek( fp, 0, SEEK_END );
		*sz = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		buffer = ( char* )gs_malloc( *sz + 1 );
		if ( buffer )
		{
			fread( buffer, 1, *sz, fp );
		}
		fclose( fp );
		buffer[ *sz ] = '0';
	}
	return buffer;
}

gs_result sdl_write_str_to_file( const char* contents, const char* mode, usize sz, const char* output_path )
{
	FILE* fp = fopen( output_path, mode );
	if ( fp ) 
	{
		s32 ret = fwrite( contents, sizeof( u8 ), sz, fp );
		if ( ret == sz )
		{
			return gs_result_success;
		}
	}
	return gs_result_failure;
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
	platform->delay		= &sdl_platform_delay;

	/*============================
	// Platform UUID
	============================*/
	platform->generate_uuid 	= &sdl_generate_uuid;
	platform->uuid_to_string 	= &sdl_uuid_to_string;
	platform->hash_uuid 		= &sdl_hash_uuid;

	/*============================
	// Platform Input
	============================*/
	platform->create_input 		= &sdl_create_input;
	platform->process_input 	= &sdl_process_input;

	platform->was_key_down 		= &sdl_was_key_down;
	platform->key_pressed 		= &sdl_key_pressed;
	platform->key_down 			= &sdl_key_down;
	platform->key_released 	 	= &sdl_key_released;

	platform->was_mouse_down 	= &sdl_was_mouse_down;
	platform->mouse_pressed 	= &sdl_mouse_pressed;
	platform->mouse_down 		= &sdl_mouse_down;
	platform->mouse_released 	= &sdl_mouse_released;

	platform->mouse_delta 			= &sdl_mouse_delta;
	platform->mouse_position 		= &sdl_mouse_position;
	platform->mouse_position_x_y 	= &sdl_mouse_position_x_y;
	platform->mouse_wheel 			= &sdl_mouse_wheel;

	platform->press_key 	= &sdl_press_key;
	platform->release_key   = &sdl_release_key;

	/*============================
	// Platform Window
	============================*/
	platform->create_window 		= &sdl_create_window;
	platform->window_swap_buffer 	= &sdl_window_swap_buffer;
	platform->window_size 			= &sdl_window_size;
	platform->window_size_w_h 		= &sdl_window_size_w_h;
	platform->set_cursor 			= &sdl_set_cursor;

	/*============================
	// Platform File IO
	============================*/
	platform->read_file_contents_into_string_null_term 	= &sdl_read_file_contents_into_string_null_term;
	platform->write_str_to_file 						= &sdl_write_str_to_file;

	// Todo(John): Remove this from the default initialization and make it a part of a plugin or config setting
	platform->settings.video.driver = gs_platform_video_driver_type_opengl;

	return platform;
}




