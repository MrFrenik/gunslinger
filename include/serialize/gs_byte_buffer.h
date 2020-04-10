#ifndef __GS_BYTE_BUFFER_H__
#define __GS_BYTE_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "math/gs_math.h"

#define gs_byte_buffer_default_capacity 	1024

/* Byte buffer */
typedef struct gs_byte_buffer
{
	u8* buffer;					// Buffer that actually holds all relevant byte data
	u32 size;					// Current size of the stored buffer data
	u32 position;				// Current read/write position in the buffer
	u32 capacity;				// Current max capacity for the buffer
} gs_byte_buffer;

// Generic "read" function for a byte buffer
#define gs_byte_buffer_read( bb, T )\
	__gs_byte_buffer_read_##T( bb )

// Generic "write" function for a byte buffer
#define gs_byte_buffer_write( bb, T, val )\
do {\
	gs_byte_buffer* _buffer = bb;\
	usize sz = sizeof( T );\
	usize total_write_size = _buffer->position + sz;\
	if ( total_write_size >= _buffer->capacity )\
	{\
		usize capacity = _buffer->capacity * 2;\
		while( capacity < total_write_size )\
		{\
			capacity *= 2;\
		}\
		gs_byte_buffer_resize( _buffer, capacity );\
	}\
	*( T* )( _buffer->buffer + _buffer->position ) = val;\
	_buffer->position += sz;\
	_buffer->size += sz;\
} while( 0 )

void gs_byte_buffer_init( gs_byte_buffer* buffer );
gs_byte_buffer gs_byte_buffer_new();
void gs_byte_buffer_free( gs_byte_buffer* buffer );
void gs_byte_buffer_clear( gs_byte_buffer* buffer );
void gs_byte_buffer_resize( gs_byte_buffer* buffer, usize sz );
void gs_byte_buffer_seek_to_beg( gs_byte_buffer* buffer );
void gs_byte_buffer_seek_to_end( gs_byte_buffer* buffer );
void gs_byte_buffer_advance_position( gs_byte_buffer* buffer, usize sz );
void gs_byte_buffer_write_str( gs_byte_buffer* buffer, const char* str );		// Expects a null terminated string
void gs_byte_buffer_read_str( gs_byte_buffer* buffer, char* str );				// Expects an allocated string
gs_result gs_byte_buffer_write_to_file( gs_byte_buffer* buffer, const char* output_path ); 	// Assumes that the output directory exists
gs_result gs_byte_buffer_read_from_file( gs_byte_buffer* buffer, const char* file_path );	// Assumes an allocated byte buffer

// "Generic" read functions
#define __gs_byte_buffer_func( T )\
	T __gs_byte_buffer_read_##T( gs_byte_buffer* bb )

__gs_byte_buffer_func( usize );
__gs_byte_buffer_func( s8 );
__gs_byte_buffer_func( u8 );
__gs_byte_buffer_func( s16 );
__gs_byte_buffer_func( u16 );
__gs_byte_buffer_func( s32 );
__gs_byte_buffer_func( u32 );
__gs_byte_buffer_func( b32 );
__gs_byte_buffer_func( s64 );
__gs_byte_buffer_func( u64 );
__gs_byte_buffer_func( f32 );
__gs_byte_buffer_func( f64 );
__gs_byte_buffer_func( gs_vec2 );
__gs_byte_buffer_func( gs_vec3 );
__gs_byte_buffer_func( gs_vec4 );
__gs_byte_buffer_func( gs_quat );
__gs_byte_buffer_func( gs_mat4 );
__gs_byte_buffer_func( gs_vqs );

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_BYTE_BUFFER_H__





