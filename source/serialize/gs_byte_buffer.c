#include "serialize/gs_byte_buffer.h"
#include "common/gs_types.h"
#include "common/gs_util.h"

void gs_byte_buffer_init(gs_byte_buffer_t* buffer)
{
	buffer->data 			= gs_malloc(gs_byte_buffer_default_capacity);
	buffer->capacity 		= gs_byte_buffer_default_capacity;
	buffer->size 			= 0;
	buffer->position 		= 0;
}

gs_byte_buffer_t gs_byte_buffer_new()
{
	gs_byte_buffer_t buffer;
	gs_byte_buffer_init(&buffer);
	return buffer;
}

void gs_byte_buffer_free(gs_byte_buffer_t* buffer)
{
	if (buffer && buffer->data) {
		gs_free(buffer->data);
	}
}

void gs_byte_buffer_clear(gs_byte_buffer_t* buffer)
{
	buffer->size = 0;
	buffer->position = 0;	
}

void gs_byte_buffer_resize(gs_byte_buffer_t* buffer, usize sz)
{
	u8* data = gs_realloc(buffer->data, sz);

	if (data == NULL) {
		return;
	}

	buffer->data = data;	
	buffer->capacity = sz;
}

void gs_byte_buffer_seek_to_beg(gs_byte_buffer_t* buffer)
{
	buffer->position = 0;
}

void gs_byte_buffer_seek_to_end(gs_byte_buffer_t* buffer)
{
	buffer->position = buffer->size;
}

void gs_byte_buffer_advance_position(gs_byte_buffer_t* buffer, usize sz)
{
	buffer->position += sz;	
}

void gs_byte_buffer_bulk_write(gs_byte_buffer_t* buffer, void* src, u32 size)
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
	memcpy((buffer->data + buffer->position), src, size);

	buffer->size += size;
	buffer->position += size;
}

void gs_byte_buffer_bulk_read(gs_byte_buffer_t* buffer, void* dst, u32 size)
{
	memcpy(dst, (buffer->data + buffer->position), size);
	buffer->position += size;
}

void gs_byte_buffer_write_str(gs_byte_buffer_t* buffer, const char* str)
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

void gs_byte_buffer_read_str(gs_byte_buffer_t* buffer, char* str)
{
	// Read in size of string from buffer
	u16 sz;
	gs_byte_buffer_read(buffer, u16, &sz);

	u32 i;
	for (i = 0; i < sz; ++i)
	{
		gs_byte_buffer_read(buffer, u8, &str[i]);
	}
	str[i] = '\0';
}

gs_result 
gs_byte_buffer_write_to_file
(
	gs_byte_buffer_t* buffer, 
	const char* output_path 
)
{
	FILE* fp = fopen(output_path, "wb");
	if (fp) 
	{
		s32 ret = fwrite(buffer->data, sizeof(u8), buffer->size, fp);
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
	gs_byte_buffer_t* buffer, 
	const char* file_path 
)
{
	buffer->data = (u8*)gs_read_file_contents_into_string_null_term(file_path, "rb", (usize*)&buffer->size);
	if (!buffer->data) {
		gs_assert(false);	
		return gs_result_failure;
	}
	buffer->position = 0;
	buffer->capacity = buffer->size;
	return gs_result_success;
}






