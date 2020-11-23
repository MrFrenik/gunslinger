#include "serialize/gs_byte_buffer.h"
#include "common/gs_util.h"

void gs_byte_buffer_init(gs_byte_buffer* buffer)
{
	buffer->buffer 			= gs_malloc(gs_byte_buffer_default_capacity);
	buffer->capacity 		= gs_byte_buffer_default_capacity;
	buffer->size 			= 0;
	buffer->position 		= 0;
}

gs_byte_buffer gs_byte_buffer_new()
{
	gs_byte_buffer buffer;
	gs_byte_buffer_init(&buffer);
	return buffer;
}

void gs_byte_buffer_free(gs_byte_buffer* buffer)
{
	if (buffer && buffer->buffer) {
		gs_free(buffer->buffer);
	}
}

void gs_byte_buffer_clear(gs_byte_buffer* buffer)
{
	buffer->size = 0;
	buffer->position = 0;	
}

void gs_byte_buffer_resize(gs_byte_buffer* buffer, usize sz)
{
	u8* data = gs_realloc(buffer->buffer, sz);

	if (data == NULL) {
		return;
	}

	buffer->buffer = data;	
	buffer->capacity = sz;
}

void gs_byte_buffer_seek_to_beg(gs_byte_buffer* buffer)
{
	buffer->position = 0;
}

void gs_byte_buffer_seek_to_end(gs_byte_buffer* buffer)
{
	buffer->position = buffer->size;
}

void gs_byte_buffer_advance_position(gs_byte_buffer* buffer, usize sz)
{
	buffer->position += sz;	
}

void gs_byte_buffer_bulk_write(gs_byte_buffer* buffer, void* src, u32 size)
{
	// Check for necessary resize
	u32 total_write_size = buffer->position + size;
	if (total_write_size >= buffer->capacity)
	{
		usize capacity = buffer->capacity * 2;
		while(capacity <= total_write_size)
		{
			capacity *= 2;
		}

		gs_byte_buffer_resize(buffer, capacity);
	}

	// memcpy data
	memcpy((buffer->buffer + buffer->position), src, size);

	buffer->size += size;
	buffer->position += size;
}

void gs_byte_buffer_bulk_read(gs_byte_buffer* buffer, void* dst, u32 size)
{
	memcpy(dst, (buffer->buffer + buffer->position), size);
	buffer->position += size;
}

void gs_byte_buffer_write_str(gs_byte_buffer* buffer, const char* str)
{
	// Write size of string
	u32 str_len = gs_string_length(str);
	gs_byte_buffer_write(buffer, u16, str_len);

	usize i; 
	for (i = 0; i < str_len; ++i)
	{
		gs_byte_buffer_write(buffer, u8, str[i]);
	}
}

void gs_byte_buffer_read_str(gs_byte_buffer* buffer, char* str)
{
	// Read in size of string from buffer
	u16 sz = gs_byte_buffer_read(buffer, u16);

	u32 i;
	for (i = 0; i < sz; ++i)
	{
		str[i] = gs_byte_buffer_read(buffer, u8);
	}
	str[i] = '\0';
}

gs_result 
gs_byte_buffer_write_to_file
( 
	gs_byte_buffer* buffer, 
	const char* output_path 
)
{
	FILE* fp = fopen(output_path, "wb");
	if (fp) 
	{
		s32 ret = fwrite(buffer->buffer, sizeof(u8), buffer->size, fp);
		if (ret == buffer->size)
		{
			return gs_result_success;
		}
	}
	return gs_result_failure;
}

gs_result 
gs_byte_buffer_read_from_file
( 
	gs_byte_buffer* buffer, 
	const char* file_path 
)
{
	buffer->buffer = (u8*)gs_read_file_contents_into_string_null_term(file_path, "rb", (usize*)&buffer->size);
	if (!buffer->buffer) {
		gs_assert(false);	
		return gs_result_failure;
	}
	buffer->position = 0;
	buffer->capacity = buffer->size;
	return gs_result_success;
}

// "Generic" read function
#define __gs_byte_buffer_read_func(T)\
	__gs_byte_buffer_func(T)\
	{\
		gs_byte_buffer* _buffer = bb;\
		usize sz = sizeof(T);\
		T val = *(T*)(_buffer->buffer + _buffer->position);\
		_buffer->position += sz;\
		return val;\
	}

__gs_byte_buffer_read_func(usize);
__gs_byte_buffer_read_func(s8);
__gs_byte_buffer_read_func(u8);
__gs_byte_buffer_read_func(u16);
__gs_byte_buffer_read_func(s16);
__gs_byte_buffer_read_func(s32);
__gs_byte_buffer_read_func(u32);
__gs_byte_buffer_read_func(b32);
__gs_byte_buffer_read_func(s64);
__gs_byte_buffer_read_func(u64);
__gs_byte_buffer_read_func(f32);
__gs_byte_buffer_read_func(f64);
__gs_byte_buffer_read_func(gs_vec2);
__gs_byte_buffer_read_func(gs_vec3);
__gs_byte_buffer_read_func(gs_vec4);
__gs_byte_buffer_read_func(gs_quat);
__gs_byte_buffer_read_func(gs_mat4);
__gs_byte_buffer_read_func(gs_vqs);






