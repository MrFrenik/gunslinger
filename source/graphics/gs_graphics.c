#include "graphics/gs_graphics.h"
#include "graphics/gs_material.h"

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
	desc.border_color[0]	= 0.f;
	desc.border_color[1]	= 0.f;
	desc.border_color[2]	= 0.f;
	desc.border_color[3]	= 0.f;
	desc.data 				= NULL;
	desc.texture_format 	= gs_texture_format_rgba8;
	desc.width 				= 0;
	desc.height 			= 0;
	desc.num_comps 			= 0;

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

void gs_rgb_to_hsv( u8 r, u8 g, u8 b, f32* h, f32* s, f32* v )
{
	f32 fR = (f32)r / 255.f; 
	f32 fG = (f32)g / 255.f; 
	f32 fB = (f32)b / 255.f;

	f32 fCMax = gs_max(gs_max(fR, fG), fB);
	f32 fCMin = gs_min(gs_min(fR, fG), fB);
	f32 fDelta = fCMax - fCMin;

	if( fDelta > 0.f ) {

		if( fCMax == fR ) 
		{
		  *h = 60.f * (fmod(((fG - fB) / fDelta), 6));
		} 
		else if( fCMax == fG ) 
		{
		  *h = 60.f * (((fB - fR) / fDelta) + 2);
		} 
		else if( fCMax == fB ) 
		{
		  *h = 60.f * (((fR - fG) / fDelta) + 4);
		}

		if( fCMax > 0.f ) 
		{
		  *s = fDelta / fCMax;
		} 
		else 
		{
		  *s = 0;
		}

		*v = fCMax;
	} 
	else 
	{
		*h = 0;
		*s = 0;
		*v = fCMax;
	}

	if( *h < 0.f ) 
	{
		*h = 360.f + *h;
	}
}

void gs_hsv_to_rgb( f32 h, f32 s, f32 v, u8* r, u8* g, u8* b )
{
	f32 fC = v * s; // Chroma
	f32 fHPrime = fmod(h / 60.0, 6);
	f32 fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
	f32 fM = v - fC;
  
  if(0 <= fHPrime && fHPrime < 1) {
    *r = fC;
    *g = fX;
    *b = 0;
  } else if(1 <= fHPrime && fHPrime < 2) {
    *r = fX;
    *g = fC;
    *b = 0;
  } else if(2 <= fHPrime && fHPrime < 3) {
    *r = 0;
    *g = fC;
    *b = fX;
  } else if(3 <= fHPrime && fHPrime < 4) {
    *r = 0;
    *g = fX;
    *b = fC;
  } else if(4 <= fHPrime && fHPrime < 5) {
    *r = fX;
    *g = 0;
    *b = fC;
  } else if(5 <= fHPrime && fHPrime < 6) {
    *r = fC;
    *g = 0;
    *b = fX;
  } else {
    *r = 0;
    *g = 0;
    *b = 0;
  }
  
	*r += fM;
	*g += fM;
	*b += fM;
}


























