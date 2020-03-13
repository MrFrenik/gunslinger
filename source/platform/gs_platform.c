#include "platform/gs_platform.h"
#include "base/gs_engine.h"

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
		.x = input->mouse.position.x, 
		.y = input->mouse.position.y
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

