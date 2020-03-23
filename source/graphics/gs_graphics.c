#include "graphics/gs_graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

gs_texture_parameter_desc gs_texture_parameter_desc_default()
{
	gs_texture_parameter_desc desc = {0};
	
	desc.texture_wrap_s 	= gs_repeat;
	desc.texture_wrap_t 	= gs_repeat;
	desc.min_filter 		= gs_linear;
	desc.mag_filter 		= gs_linear;
	desc.mipmap_filter 		= gs_linear;
	desc.generate_mips 		= true;
	desc.generate_mips 		= true;
	desc.border_color[0]	= 0.f;
	desc.border_color[1]	= 0.f;
	desc.border_color[2]	= 0.f;
	desc.border_color[3]	= 0.f;
	desc.data 			= NULL;
	desc.texture_format = gs_texture_format_ldr;
	desc.width 			= 0;
	desc.height 		= 0;
	desc.num_comps 		= 0;

	return desc;
}

void* gs_load_texture_data_from_file( const char* file_path, b32 flip_vertically_on_load )
{
// Load texture data
stbi_set_flip_vertically_on_load( flip_vertically_on_load );

// For now, this data will always have 4 components, since STBI_rgb_alpha is being passed in as required components param
// Could optimize this later
s32 width, height, num_comps;
void* texture_data = stbi_load( file_path, ( s32* )&width, ( s32* )&height, &num_comps, STBI_rgb_alpha );

if ( !texture_data )
{
	gs_println( "Warning: could not load texture: %s", file_path );
	return NULL;
}

return texture_data;
}
