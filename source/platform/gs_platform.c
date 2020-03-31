#include "platform/gs_platform.h"
#include "base/gs_engine.h"

/*============================
// Platform Window
============================*/

gs_resource_handle __gs_platform_create_window( const char* title, u32 width, u32 height )
{
	struct gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	void* win = platform->create_window_internal( title, width, height );
	gs_assert( win );
	gs_resource_handle handle = gs_slot_array_insert( platform->windows, win );
	gs_dyn_array_push( platform->active_window_handles, handle );
	return handle;
}

gs_resource_handle __gs_platform_main_window()
{
	// Should be the first element of the slot array...Great assumption to make.
	return 0;
}

/*============================
// Platform Input
============================*/

#define __input()\
	( &gs_engine_instance()->ctx.platform->input )

void __gs_platform_update_input()
{
	gs_platform_input* input = __input();

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
}

b32 __gs_platform_was_key_down( gs_platform_keycode code )
{
	gs_platform_input* input = __input();
	return ( input->prev_key_map[ code ] );
}

b32 __gs_platform_key_down( gs_platform_keycode code )
{
	gs_platform_input* input = __input();
	return ( input->key_map[ code ] );
}

b32 __gs_platform_key_pressed( gs_platform_keycode code )
{
	gs_platform_input* input = __input();
	if ( __gs_platform_key_down( code ) && !__gs_platform_was_key_down( code ) )
	{
		return true;
	}
	return false;
}

b32 __gs_platform_key_released( gs_platform_keycode code )
{
	gs_platform_input* input = __input();
	return ( __gs_platform_was_key_down( code ) && !__gs_platform_key_down( code ) );
}

b32 __gs_platform_was_mouse_down( gs_platform_mouse_button_code code )
{
	gs_platform_input* input = __input();
	return ( input->mouse.prev_button_map[ code ] );
}

void __gs_platform_press_mouse_button( gs_platform_mouse_button_code code )
{
	gs_platform_input* input = __input();
	if ( (u32)code < (u32)gs_mouse_button_code_count ) 
	{
		input->mouse.button_map[ code ] = true;
	}
}

void __gs_platform_release_mouse_button( gs_platform_mouse_button_code code )
{
	gs_platform_input* input = __input();
	if ( (u32)code < (u32)gs_mouse_button_code_count ) 
	{
		input->mouse.button_map[ code ] = false;
	}
}

b32 __gs_platform_mouse_down( gs_platform_mouse_button_code code )
{
	gs_platform_input* input = __input();
	return ( input->mouse.button_map[ code ] );
}

b32 __gs_platform_mouse_pressed( gs_platform_mouse_button_code code )
{
	gs_platform_input* input = __input();
	if ( __gs_platform_mouse_down( code ) && !__gs_platform_was_mouse_down( code ) )
	{
		return true;
	}
	return false;
}

b32 __gs_platform_mouse_released( gs_platform_mouse_button_code code )
{
	gs_platform_input* input = __input();
	return ( __gs_platform_was_mouse_down( code ) && !__gs_platform_mouse_down( code ) );
}

gs_vec2 __gs_platform_mouse_delta()
{
	gs_platform_input* input = __input();

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

gs_vec2 __gs_platform_mouse_position()
{
	gs_platform_input* input = __input();

	return ( gs_vec2 ) 
	{
		input->mouse.position.x, 
		input->mouse.position.y
	};
}

void __gs_platform_mouse_position_x_y( f32* x, f32* y )
{
	gs_platform_input* input = __input();
	*x = input->mouse.position.x;
	*y = input->mouse.position.y;
}

void __gs_platform_mouse_wheel( f32* x, f32* y )
{
	gs_platform_input* input = __input();
	*x = input->mouse.wheel.x;
	*y = input->mouse.wheel.y;	
}

void __gs_platform_press_key( gs_platform_keycode code )
{
	gs_platform_input* input = __input();
	if ( code < gs_keycode_count ) 
	{
		input->key_map[ code ] = true;
	}
}

void __gs_platform_release_key( gs_platform_keycode code )
{
	gs_platform_input* input = __input();
	if ( code < gs_keycode_count ) 
	{
		input->key_map[ code ] = false;
	}
}

/*============================
// Platform File I/O
============================*/

char* __gs_platform_read_file_contents_into_string_null_term( const char* file_path, const char* mode, usize* sz )
{
	char* buffer = 0;
	FILE* fp = fopen( file_path, mode );
	usize _sz = 0;
	if ( fp )
	{
		fseek( fp, 0, SEEK_END );
		_sz = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		buffer = ( char* )gs_malloc( _sz );
		if ( buffer )
		{
			fread( buffer, 1, _sz, fp );
		}
		fclose( fp );
		buffer[ _sz ] = '\0';
	}
	if ( sz )
		*sz = _sz;
	return buffer;
}

gs_result __gs_platform_write_str_to_file( const char* contents, const char* mode, usize sz, const char* output_path )
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

/*============================
// Platform UUID
============================*/

struct gs_uuid __gs_platform_generate_uuid()
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
void __gs_platform_uuid_to_string( char* tmp_buffer, const struct gs_uuid* uuid )
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

u32 __gs_platform_hash_uuid( const struct gs_uuid* uuid )
{
	char temp_buffer[] = gs_uuid_temp_str_buffer();
	__gs_platform_uuid_to_string( temp_buffer, uuid );
	return ( gs_hash_str( temp_buffer ) );
}

void __gs_default_init_platform( struct gs_platform_i* platform )
{
	gs_assert( platform );

	// Just assert these for now
	__gs_verify_platform_correctness( platform );

	// Initialize random with time
	srand(time(0));

	/*============================
	// Platform Window
	============================*/
	platform->windows 				= gs_slot_array_new( gs_platform_window_ptr );
	platform->active_window_handles = gs_dyn_array_new( gs_resource_handle );
	platform->create_window 		= &__gs_platform_create_window;
	platform->main_window 			= &__gs_platform_main_window;

	/*============================
	// Platform Input
	============================*/
	platform->update_input 		= &__gs_platform_update_input;
	platform->press_key 		= &__gs_platform_press_key;
	platform->release_key   	= &__gs_platform_release_key;
	platform->was_key_down 		= &__gs_platform_was_key_down;
	platform->key_pressed 		= &__gs_platform_key_pressed;
	platform->key_down 			= &__gs_platform_key_down;
	platform->key_released 	 	= &__gs_platform_key_released;

	platform->press_mouse_button 	= &__gs_platform_press_mouse_button;
	platform->release_mouse_button 	= &__gs_platform_release_mouse_button;
	platform->was_mouse_down 		= &__gs_platform_was_mouse_down;
	platform->mouse_pressed 		= &__gs_platform_mouse_pressed;
	platform->mouse_down 			= &__gs_platform_mouse_down;
	platform->mouse_released 		= &__gs_platform_mouse_released;

	platform->mouse_delta 			= &__gs_platform_mouse_delta;
	platform->mouse_position 		= &__gs_platform_mouse_position;
	platform->mouse_position_x_y 	= &__gs_platform_mouse_position_x_y;
	platform->mouse_wheel 			= &__gs_platform_mouse_wheel;

	/*============================
	// Platform UUID
	============================*/
	platform->generate_uuid 	= &__gs_platform_generate_uuid;
	platform->uuid_to_string 	= &__gs_platform_uuid_to_string;
	platform->hash_uuid 		= &__gs_platform_hash_uuid;

	/*============================
	// Platform File IO
	============================*/

	platform->read_file_contents 	= &__gs_platform_read_file_contents_into_string_null_term;
	platform->write_str_to_file 	= &__gs_platform_write_str_to_file;

	// Default world time initialization
	platform->time.max_fps 		= 60.0;
	platform->time.current 		= 0.0;
	platform->time.delta 		= 0.0;
	platform->time.update 		= 0.0;
	platform->time.render 		= 0.0;
	platform->time.previous		= 0.0;
	platform->time.frame 		= 0.0;

	// Custom initialize plaform layer
	platform->init( platform );
}

void __gs_verify_platform_correctness( struct gs_platform_i* platform )
{
	gs_assert( platform );
	gs_assert( platform->init );
	gs_assert( platform->shutdown );
	gs_assert( platform->sleep );
	gs_assert( platform->elapsed_time );
	gs_assert( platform->process_input );
	gs_assert( platform->create_window_internal );
	gs_assert( platform->window_swap_buffer );
	gs_assert( platform->set_window_size );
	gs_assert( platform->window_size );
	gs_assert( platform->window_size_w_h );
	gs_assert( platform->set_cursor );
	gs_assert( platform->enable_vsync );
}

