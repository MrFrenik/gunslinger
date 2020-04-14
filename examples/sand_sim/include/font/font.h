#ifndef GS_SAND_SIM_FONT_H
#define GS_SAND_SIM_FONT_H

#include "gs.h"

// For this font, Each glyph is monospaced, 5 x 7 pixels.
// Total font size is 128 x 64.
typedef struct font_glyph_t
{
	u32 x;
	u32 y;
	u32 width;
	u32 height;
} font_glyph_t;

// 6 rows of font data to use * 18 columns
// Total num glyphs = 6 * 18 - (18 - 5)
#define total_num_font_glyphs (6 * 18)

typedef struct font_t
{
	void* data;
	u32 width;
	u32 height;
	u32 num_comps;
	u32 glyph_advance;
	gs_texture_format texture_format;
	font_glyph_t glyphs[ total_num_font_glyphs ];
} font_t;

#endif