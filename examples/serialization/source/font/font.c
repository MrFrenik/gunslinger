#include "font.h"
#include "font/font_data.c"

font_t construct_font_data()
{
	font_t f = {0};

	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	f.data = g_font_data;
	f.width = 90;
	f.height = 42;
	f.num_comps = 3;

	// Set up metrics
	f.glyph_advance = 1;

	// Construct glyph information
	const s32 glyph_width = 5, glyph_height = 7;
	for ( u32 r = 0; r < 6; ++r ) 
	{
		for ( u32 c = 0; c < 18; ++c ) 
		{
			u32 idx = r * 18 + c;
			f.glyphs[ idx ] = (font_glyph_t){ c * 5, r * 7, 5, 7 };
		}
	}

	return f;
}

font_glyph_t get_glyph( font_t* f, char c )
{
	switch ( c ) 
	{
		case ' ': return f->glyphs[ 0 ];
		case '!': return f->glyphs[ 1 ];
		case '"': return f->glyphs[ 2 ];
		case '#': return f->glyphs[ 3 ];
		case '$': return f->glyphs[ 4 ];
		case '%': return f->glyphs[ 5 ];
		case '&': return f->glyphs[ 6 ];
		case '\'': return f->glyphs[ 7 ];
		case '(': return f->glyphs[ 8 ];
		case ')': return f->glyphs[ 9 ];
		case '*': return f->glyphs[ 10 ];
		case '+': return f->glyphs[ 11 ];
		case ',': return f->glyphs[ 12 ];
		case '-': return f->glyphs[ 13 ];
		case '.': return f->glyphs[ 14 ];
		case '/': return f->glyphs[ 15 ];
		case '0': return f->glyphs[ 16 ];
		case '1': return f->glyphs[ 17 ];
		case '2': return f->glyphs[ 18 ];
		case '3': return f->glyphs[ 19 ];
		case '4': return f->glyphs[ 20 ];
		case '5': return f->glyphs[ 21 ];
		case '6': return f->glyphs[ 22 ];
		case '7': return f->glyphs[ 23 ];
		case '8': return f->glyphs[ 24 ];
		case '9': return f->glyphs[ 25 ];
		case ':': return f->glyphs[ 26 ];
		case ';': return f->glyphs[ 27 ];
		case '<': return f->glyphs[ 28 ];
		case '=': return f->glyphs[ 29 ];
		case '>': return f->glyphs[ 30 ];
		case '?': return f->glyphs[ 31 ];
		case '@': return f->glyphs[ 32 ];
		case 'A': return f->glyphs[ 33 ];
		case 'B': return f->glyphs[ 34 ];
		case 'C': return f->glyphs[ 35 ];
		case 'D': return f->glyphs[ 36 ];
		case 'E': return f->glyphs[ 37 ];
		case 'F': return f->glyphs[ 38 ];
		case 'G': return f->glyphs[ 39 ];
		case 'H': return f->glyphs[ 40 ];
		case 'I': return f->glyphs[ 41 ];
		case 'J': return f->glyphs[ 42 ];
		case 'K': return f->glyphs[ 43 ];
		case 'L': return f->glyphs[ 44 ];
		case 'M': return f->glyphs[ 45 ];
		case 'N': return f->glyphs[ 46 ];
		case 'O': return f->glyphs[ 47 ];
		case 'P': return f->glyphs[ 48 ];
		case 'Q': return f->glyphs[ 49 ];
		case 'R': return f->glyphs[ 50 ];
		case 'S': return f->glyphs[ 51 ];
		case 'T': return f->glyphs[ 52 ];
		case 'U': return f->glyphs[ 53 ];
		case 'V': return f->glyphs[ 54 ];
		case 'W': return f->glyphs[ 55 ];
		case 'X': return f->glyphs[ 56 ];
		case 'Y': return f->glyphs[ 57 ];
		case 'Z': return f->glyphs[ 58 ];
		case '[': return f->glyphs[ 59 ];
		case '\\': return f->glyphs[ 60 ];
		case ']': return f->glyphs[ 61 ];
		case '^': return f->glyphs[ 62 ];
		case '_': return f->glyphs[ 63 ];
		case '`': return f->glyphs[ 64 ];
		case 'a': return f->glyphs[ 65 ];
		case 'b': return f->glyphs[ 66 ];
		case 'c': return f->glyphs[ 67 ];
		case 'd': return f->glyphs[ 68 ];
		case 'e': return f->glyphs[ 69 ];
		case 'f': return f->glyphs[ 70 ];
		case 'g': return f->glyphs[ 71 ];
		case 'h': return f->glyphs[ 72 ];
		case 'i': return f->glyphs[ 73 ];
		case 'j': return f->glyphs[ 74 ];
		case 'k': return f->glyphs[ 75 ];
		case 'l': return f->glyphs[ 76 ];
		case 'm': return f->glyphs[ 77 ];
		case 'n': return f->glyphs[ 78 ];
		case 'o': return f->glyphs[ 79 ];
		case 'p': return f->glyphs[ 80 ];
		case 'q': return f->glyphs[ 81 ];
		case 'r': return f->glyphs[ 82 ];
		case 's': return f->glyphs[ 83 ];
		case 't': return f->glyphs[ 84 ];
		case 'u': return f->glyphs[ 85 ];
		case 'v': return f->glyphs[ 86 ];
		case 'w': return f->glyphs[ 87 ];
		case 'x': return f->glyphs[ 88 ];
		case 'y': return f->glyphs[ 89 ];
		case 'z': return f->glyphs[ 90 ];
		case '{': return f->glyphs[ 91 ];
		case '|': return f->glyphs[ 92 ];
		case '}': return f->glyphs[ 93 ];
		case '~': return f->glyphs[ 94 ];

		// For anything not supported, just return empty space
		default: {
			return (font_glyph_t){0};
		} break;
	}
}
