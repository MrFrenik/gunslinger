#ifndef __GS_BYTE_BUFFER_H__
#define __GS_BYTE_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "math/gs_math.h"

#define gs_byte_buffer_default_capacity 	1024

/* Byte buffer */
typedef struct gs_byte_buffer_t
{
	u8* data;					// Buffer that actually holds all relevant byte data
	u32 size;					// Current size of the stored buffer data
	u32 position;				// Current read/write position in the buffer
	u32 capacity;				// Current max capacity for the buffer
} gs_byte_buffer_t;

// Generic "write" function for a byte buffer
#define gs_byte_buffer_write(bb, T, val)\
do {\
	gs_byte_buffer_t* _buffer = bb;\
	usize sz = sizeof(T);\
	usize total_write_size = _buffer->position + sz;\
	if (total_write_size >= _buffer->capacity)\
	{\
		usize capacity = _buffer->capacity * 2;\
		while(capacity < total_write_size)\
		{\
			capacity *= 2;\
		}\
		gs_byte_buffer_resize(_buffer, capacity);\
	}\
	*(T*)(_buffer->data + _buffer->position) = val;\
	_buffer->position += sz;\
	_buffer->size += sz;\
} while (0)

// Generic "read" function
#define gs_byte_buffer_read(_buffer, T, _val_p)\
do {\
	T* _v = (T*)(_val_p);\
	gs_byte_buffer_t* _bb = (_buffer);\
	*(_v) = *(T*)(_bb->data + _bb->position);\
	_bb->position += sizeof(T);\
} while (0)

// Defines variable and sets value from buffer in place
// Use to construct a new variable
#define gs_byte_buffer_readc(_buffer, T, name)\
	T name = gs_default_val();\
	gs_byte_buffer_read((_buffer), T, &name);

void gs_byte_buffer_init(gs_byte_buffer_t* buffer);
gs_byte_buffer_t gs_byte_buffer_new();
void gs_byte_buffer_free(gs_byte_buffer_t* buffer);
void gs_byte_buffer_clear(gs_byte_buffer_t* buffer);
void gs_byte_buffer_resize(gs_byte_buffer_t* buffer, usize sz);
void gs_byte_buffer_seek_to_beg(gs_byte_buffer_t* buffer);
void gs_byte_buffer_seek_to_end(gs_byte_buffer_t* buffer);
void gs_byte_buffer_advance_position(gs_byte_buffer_t* buffer, usize sz);
void gs_byte_buffer_write_str(gs_byte_buffer_t* buffer, const char* str);		// Expects a null terminated string
void gs_byte_buffer_read_str(gs_byte_buffer_t* buffer, char* str);				// Expects an allocated string
void gs_byte_buffer_bulk_write(gs_byte_buffer_t* buffer, void* src, u32 sz);
void gs_byte_buffer_bulk_read(gs_byte_buffer_t* buffer, void* dst, u32 sz);
gs_result gs_byte_buffer_write_to_file(gs_byte_buffer_t* buffer, const char* output_path); 	// Assumes that the output directory exists
gs_result gs_byte_buffer_read_from_file(gs_byte_buffer_t* buffer, const char* file_path);	// Assumes an allocated byte buffer

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_BYTE_BUFFER_H__





