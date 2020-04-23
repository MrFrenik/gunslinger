#include <gs.h>

#include "render_pass/blur_pass.h"
#include "render_pass/bright_filter_pass.h"
#include "render_pass/composite_pass.h"
#include "font/font.h"
#include "font/font_data.c"

#if (defined GS_PLATFORM_APPLE)
	_global const s32 g_window_width 	= 800;
	_global const s32 g_window_height 	= 600;
#else
	_global const s32 g_window_width 	= 1258;
	_global const s32 g_window_height 	= 848;
#endif

_global const s32 g_texture_width 	= 1258 / 2;
_global const s32 g_texture_height 	= 848 / 2;

// 32 bit color structure
typedef struct color_t
{
	u8 r;
	u8 g;
	u8 b;
	u8 a;
} color_t;

typedef struct particle_t
{
	u8 id;
	f32 life_time;
	gs_vec2 velocity;
	color_t color;
	b32 has_been_updated_this_frame;
} particle_t;

// Should have a hash map of glyph character to glyph metric

// Globals
_global gs_resource( gs_vertex_buffer ) 	g_vbo = {0};
_global gs_resource( gs_index_buffer ) 		g_ibo = {0};
_global gs_resource( gs_command_buffer ) 	g_cb = {0};
_global gs_resource( gs_shader ) 			g_shader = {0};
_global gs_resource( gs_uniform ) 			u_tex = {0}; 
_global gs_resource( gs_uniform ) 			u_flip_y = {0}; 
_global gs_resource( gs_texture ) 			g_tex = {0};
_global gs_resource( gs_texture ) 			g_tex_ui = {0};
_global gs_resource( gs_texture ) 			g_rt = {0};
_global gs_resource( gs_frame_buffer ) 		g_fb = {0};
_global blur_pass_t 						g_blur_pass = {0};
_global bright_filter_pass_t 				g_bright_pass = {0};
_global composite_pass_t 					g_composite_pass = {0};
_global font_t								g_font = {0};

// For now, all particle information will simply be a value to determine its material id
#define mat_id_empty (u8)0
#define mat_id_sand  (u8)1
#define mat_id_water (u8)2
#define mat_id_salt (u8)3
#define mat_id_wood (u8)4
#define mat_id_fire (u8)5
#define mat_id_smoke (u8)6
#define mat_id_ember (u8)7
#define mat_id_steam (u8)8
#define mat_id_gunpowder (u8)9
#define mat_id_oil (u8)10
#define mat_id_lava (u8)11
#define mat_id_stone (u8)12
#define mat_id_acid (u8)13

// Colors
#define mat_col_empty (color_t){ 0, 0, 0, 0}
#define mat_col_sand  (color_t){ 150, 100, 50, 255 }
#define mat_col_salt  (color_t){ 200, 180, 190, 255 }
#define mat_col_water (color_t){ 20, 100, 170, 200 }
#define mat_col_stone (color_t){ 120, 110, 120, 255 }
#define mat_col_wood (color_t){ 60, 40, 20, 255 }
#define mat_col_fire  (color_t){ 150, 20, 0, 255 }
#define mat_col_smoke (color_t){ 50, 50, 50, 255 }
#define mat_col_ember (color_t){ 200, 120, 20, 255 }
#define mat_col_steam (color_t){ 220, 220, 250, 255 }
#define mat_col_gunpowder (color_t){ 60, 60, 60, 255 }
#define mat_col_oil (color_t){ 80, 70, 60, 255 }
#define mat_col_lava  (color_t){ 200, 50, 0, 255 }
#define mat_col_acid  (color_t){ 90, 200, 60, 255 }

typedef enum material_selection
{
	mat_sel_sand = 0x00,
	mat_sel_water,
	mat_sel_salt,
	mat_sel_wood,
	mat_sel_fire,
	mat_sel_smoke,
	mat_sel_steam,
	mat_sel_gunpowder,
	mat_sel_oil,
	mat_sel_lava,
	mat_sel_stone,
	mat_sel_acid
} material_selection;

// Material selection for "painting" / default to sand
_global material_selection g_material_selection = mat_sel_sand;

// World update processing structure
_global u8* g_world_process_update_structure = {0};	// Every particle has id associated with it? Jeezuz...

// World particle data structure
_global particle_t* g_world_particle_data = {0};

// Texture buffers
_global color_t* g_texture_buffer = {0};

// UI texture buffer
_global color_t* g_ui_buffer = {0};

// Frame counter
_global u32 g_frame_counter = 0;

// World physics settings
_global f32 gravity = 10.f;

_global f32 g_selection_radius = 10.f;

_global b32 g_show_material_selection_panel = true;
_global b32 g_run_simulation = true;
_global b32 g_show_frame_count = true;
_global b32 g_use_post_processing = true;

// Handle for main window
_global gs_resource_handle g_window;

const char* v_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec2 a_pos;\n"
"layout (location = 1) in vec2 a_texCoord;\n"
"uniform int u_flip_y;"
"out vec2 texCoord;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(a_pos, 0.0, 1.0);\n"
"	texCoord = vec2(a_texCoord.x, bool(u_flip_y) ? 1.0 - a_texCoord.y : a_texCoord.y);\n"
"}";

const char* f_src = "\n"
"#version 330 core\n"
"out vec4 frag_color;\n"
"in vec2 texCoord;\n"
"uniform sampler2D u_tex;\n"
"void main()\n"
"{\n"
"	frag_color = texture(u_tex, texCoord);\n"
"}";

// Forward Decls.
gs_result app_init();
gs_result app_update();
gs_result app_shutdown();

particle_t particle_empty();
particle_t particle_sand();
particle_t particle_water();
particle_t particle_salt();
particle_t particle_wood();
particle_t particle_fire();
particle_t particle_lava();
particle_t particle_smoke();
particle_t particle_ember();
particle_t particle_steam();
particle_t particle_gunpowder();
particle_t particle_oil();
particle_t particle_stone();
particle_t particle_acid();

void update_input();
b32 update_ui();

// Particle updates
void update_particle_sim();
void update_sand( u32 x, u32 y );
void update_water( u32 x, u32 y );
void update_salt( u32 x, u32 y );
void update_fire( u32 x, u32 y );
void update_smoke( u32 x, u32 y );
void update_ember( u32 x, u32 y );
void update_steam( u32 x, u32 y );
void update_gunpowder( u32 x, u32 y );
void update_oil( u32 x, u32 y );
void update_lava( u32 x, u32 y );
void update_acid( u32 x, u32 y );
void update_default( u32 x, u32 y );

// Utilities for writing data into color buffer
void write_data( u32 idx, particle_t );

// Rendering
void render_scene();

// Font methods
void construct_font_data();
font_glyph_t get_glyph( font_t* f, char c );

gs_vec2 calculate_mouse_position()
{
	gs_vec2 ws = gs_engine_instance()->ctx.platform->window_size( g_window );
	gs_vec2 pmp = gs_engine_instance()->ctx.platform->mouse_position();
	// Need to place mouse into frame
	f32 x_scale = pmp.x / (f32)ws.x;
	f32 y_scale = pmp.y / (f32)ws.y;
	return (gs_vec2){ x_scale * (f32)g_texture_width, y_scale * (f32)g_texture_height };
}

s32 random_val( s32 lower, s32 upper )
{
	if ( upper < lower ) {
		s32 tmp = lower;
		lower = upper;
		upper = tmp;
	}
	return ( rand() % (upper - lower + 1) + lower );
}

s32 compute_idx( s32 x, s32 y )
{
	return ( y * g_texture_width + x );
}

b32 in_bounds( s32 x, s32 y )
{
	if ( x < 0 || x > (g_texture_width - 1) || y < 0 || y > (g_texture_height - 1) ) return false;
	return true;
}

b32 is_empty( s32 x, s32 y )
{
	return ( in_bounds( x, y ) && g_world_particle_data[ compute_idx( x, y ) ].id == mat_id_empty );
}

particle_t get_particle_at( s32 x, s32 y )
{
	return g_world_particle_data[ compute_idx( x, y ) ];
}

b32 completely_surrounded( s32 x, s32 y ) 
{
	// Top
	if ( in_bounds( x, y - 1 ) && !is_empty( x, y - 1 ) ) {
		return false;
	}
	// Bottom
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) ) {
		return false;
	}
	// Left
	if ( in_bounds( x - 1, y ) && !is_empty( x - 1, y ) ) {
		return false;
	}
	// Right
	if ( in_bounds( x + 1, y ) && !is_empty( x + 1, y ) ) {
		return false;
	}
	// Top Left
	if ( in_bounds( x - 1, y - 1 ) && !is_empty( x - 1, y - 1 ) ) {
		return false;
	}
	// Top Right
	if ( in_bounds( x + 1, y - 1 ) && !is_empty( x + 1, y - 1 ) ) {
		return false;
	}
	// Bottom Left
	if ( in_bounds( x - 1, y + 1 ) && !is_empty( x - 1, y + 1 ) ) {
		return false;
	}
	// Bottom Right
	if ( in_bounds( x + 1, y + 1 ) && !is_empty( x + 1, y + 1 ) ) {
		return false;
	}

	return true;
}

b32 is_in_liquid( s32 x, s32 y, s32* lx, s32* ly ) 
{
	if ( in_bounds( x, y ) && (get_particle_at( x, y ).id == mat_id_water || get_particle_at( x, y ).id == mat_id_oil) ) {
		*lx = x; *ly = y;
		return true;
	}
	if ( in_bounds( x, y - 1 ) && (get_particle_at( x, y - 1 ).id == mat_id_water || get_particle_at( x, y - 1 ).id == mat_id_oil) ) {
		*lx = x; *ly = y - 1;
		return true;
	}
	if ( in_bounds( x, y + 1 ) && (get_particle_at( x, y + 1 ).id == mat_id_water || get_particle_at( x, y + 1 ).id == mat_id_oil) ) {
		*lx = x; *ly = y + 1;
		return true;
	}
	if ( in_bounds( x - 1, y ) && (get_particle_at( x - 1, y ).id == mat_id_water || get_particle_at( x - 1, y ).id == mat_id_oil) ) {
		*lx = x - 1; *ly = y;
		return true;
	}
	if ( in_bounds( x - 1, y - 1 ) && (get_particle_at( x - 1, y - 1 ).id == mat_id_water || get_particle_at( x - 1, y - 1 ).id == mat_id_oil) ) {
		*lx = x - 1; *ly = y - 1;
		return true;
	}
	if ( in_bounds( x - 1, y + 1 ) && (get_particle_at( x - 1, y + 1 ).id == mat_id_water || get_particle_at( x - 1, y + 1 ).id == mat_id_oil) ) {
		*lx = x - 1; *ly = y + 1;
		return true;
	}
	if ( in_bounds( x + 1, y ) && (get_particle_at( x + 1, y ).id == mat_id_water || get_particle_at( x + 1, y ).id == mat_id_oil) ) {
		*lx = x + 1; *ly = y;
		return true;
	}
	if ( in_bounds( x + 1, y - 1 ) && (get_particle_at( x + 1, y - 1 ).id == mat_id_water || get_particle_at( x + 1, y - 1 ).id == mat_id_oil) ) {
		*lx = x + 1; *ly = y - 1;
		return true;
	}
	if ( in_bounds( x + 1, y + 1 ) && (get_particle_at( x + 1, y + 1 ).id == mat_id_water || get_particle_at( x + 1, y + 1 ).id == mat_id_oil) ) {
		*lx = x + 1; *ly = y + 1;
		return true;
	}
	return false;
}

b32 is_in_water( s32 x, s32 y, s32* lx, s32* ly ) 
{
	if ( in_bounds( x, y ) && (get_particle_at( x, y ).id == mat_id_water) ) {
		*lx = x; *ly = y;
		return true;
	}
	if ( in_bounds( x, y - 1 ) && (get_particle_at( x, y - 1 ).id == mat_id_water) ) {
		*lx = x; *ly = y - 1;
		return true;
	}
	if ( in_bounds( x, y + 1 ) && (get_particle_at( x, y + 1 ).id == mat_id_water) ) {
		*lx = x; *ly = y + 1;
		return true;
	}
	if ( in_bounds( x - 1, y ) && (get_particle_at( x - 1, y ).id == mat_id_water) ) {
		*lx = x - 1; *ly = y;
		return true;
	}
	if ( in_bounds( x - 1, y - 1 ) && (get_particle_at( x - 1, y - 1 ).id == mat_id_water) ) {
		*lx = x - 1; *ly = y - 1;
		return true;
	}
	if ( in_bounds( x - 1, y + 1 ) && (get_particle_at( x - 1, y + 1 ).id == mat_id_water) ) {
		*lx = x - 1; *ly = y + 1;
		return true;
	}
	if ( in_bounds( x + 1, y ) && (get_particle_at( x + 1, y ).id == mat_id_water) ) {
		*lx = x + 1; *ly = y;
		return true;
	}
	if ( in_bounds( x + 1, y - 1 ) && (get_particle_at( x + 1, y - 1 ).id == mat_id_water) ) {
		*lx = x + 1; *ly = y - 1;
		return true;
	}
	if ( in_bounds( x + 1, y + 1 ) && (get_particle_at( x + 1, y + 1 ).id == mat_id_water) ) {
		*lx = x + 1; *ly = y + 1;
		return true;
	}
	return false;
}

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "SandSim";
	app.window_width 		= g_window_width;
	app.window_height 		= g_window_height;
	app.init 				= &app_init;
	app.update 				= &app_update;
	app.shutdown 			= &app_shutdown;
	app.frame_rate 			= 60;
	app.enable_vsync 		= false;

	// Construct internal instance of our engine
	gs_engine* engine = gs_engine_construct( app );

	// Run the internal engine loop until completion
	gs_result res = engine->run();

	// Check result of engine after exiting loop
	if ( res != gs_result_success ) 
	{
		gs_println( "Error: Engine did not successfully finish running." );
		return -1;
	}

	gs_println( "Gunslinger exited successfully." );

	return 0;	
}

typedef struct hsv_t
{
	f32 h;
	f32 s;
	f32 v;
} hsv_t;

// From on: https://gist.github.com/fairlight1337/4935ae72bcbcc1ba5c72
hsv_t rgb_to_hsv( color_t c ) 
{
	gs_vec3 cv = (gs_vec3){ (f32)c.r / 255.f, (f32)c.g / 255.f, (f32)c.b / 255.f };
	f32 fR = cv.x, fG = cv.y, fB = cv.z;

	f32 fCMax = gs_max(gs_max(fR, fG), fB);
	f32 fCMin = gs_min(gs_min(fR, fG), fB);
	f32 fDelta = fCMax - fCMin;

	hsv_t hsv;

	if(fDelta > 0) {
		if(fCMax == fR) {
		  hsv.h = 60 * (fmod(((fG - fB) / fDelta), 6));
	} else if(fCMax == fG) {
		  hsv.h = 60 * (((fB - fR) / fDelta) + 2);
	} else if(fCMax == fB) {
		  hsv.h = 60 * (((fR - fG) / fDelta) + 4);
	}

	if(fCMax > 0) {
	  hsv.s = fDelta / fCMax;
	} else {
	  hsv.s = 0;
	}

	hsv.v = fCMax;
	} else {
		hsv.h = 0;
		hsv.s = 0;
		hsv.v = fCMax;
	}

	if(hsv.h < 0) {
		hsv.h = 360 + hsv.h;
	}

	return hsv;
}

// Implemented from: https://stackoverflow.com/questions/27374550/how-to-compare-color-object-and-get-closest-color-in-an-color
// distance between two hues:
f32 hue_dist( f32 h1, f32 h2 )
{ 
    f32 d = fabsf( h1 -  h2 ); 
    return d > 180.f ? 360.f - d : d; 
}

 // color brightness as perceived:
f32 brightness( color_t c )
{ 
	return ( (f32)c.r * 0.299f + (f32)c.g * 0.587f + (f32)c.b *0.114f ) / 256.f;
}

f32 color_num( color_t c ) 
{
	const f32 bright_factor = 100.0f;
	const f32 sat_factor = 0.1f;
	hsv_t hsv = rgb_to_hsv( c );
	return hsv.s * sat_factor + brightness( c ) * bright_factor;
}

#define __check_hsv(c0, c1, p_func)\
do {\
	hsv_t hsv0 = rgb_to_hsv( c0 );\
	hsv_t hsv1 = rgb_to_hsv( c1 );\
	f32 d = abs( color_num( c0 ) - color_num( c1 ) ) + hue_dist( hsv0.h, hsv1.h );\
	if ( d < min_dist ) {\
		min_dist = d;\
		p = p_func();\
	}\
} while ( 0 )

#define __check_dist_euclidean(c0, c1, p_func)\
	do {\
		gs_vec4 c0_vec = (gs_vec4){ (f32)c0.r, c0.g, c0.b, 255.f };\
		gs_vec4 c1_vec = (gs_vec4){ (f32)c1.r, c1.g, c1.b, 255.f };\
		f32 d = gs_vec4_dist( c0_vec, c1_vec );\
		if ( d < min_dist ) {\
			min_dist = d;\
			p = p_func();\
		}\
	} while ( 0 )

#define __check_dist( c0, c1, p_func )\
	do\
	{\
		f32 rd = (f32)c0.r - (f32)c1.r;\
		f32 gd = (f32)c0.g - (f32)c1.g;\
		f32 bd = (f32)c0.b - (f32)c1.b;\
		f32 sd = rd * rd + gd * gd + bd * bd;\
		f32 d = pow(rd * 0.299, 2) + pow(gd * 0.587, 2) + pow(bd * 0.114, 2);\
		if (d < min_dist) {\
			min_dist = d;\
			p = p_func();\
		}\
	} while ( 0 )

particle_t get_closest_particle_from_color( color_t c )
{
	particle_t p = particle_empty();
	f32 min_dist = f32_max;
	gs_vec4 c_vec = (gs_vec4){ (f32)c.r, (f32)c.g, (f32)c.b, (f32)c.a };
	u8 id = mat_id_empty;

	__check_dist_euclidean( c, mat_col_sand, particle_sand );
	__check_dist_euclidean( c, mat_col_water, particle_water );
	__check_dist_euclidean( c, mat_col_salt, particle_salt );
	__check_dist_euclidean( c, mat_col_wood, particle_wood );
	__check_dist_euclidean( c, mat_col_fire, particle_fire );
	__check_dist_euclidean( c, mat_col_smoke, particle_smoke );
	__check_dist_euclidean( c, mat_col_steam, particle_steam );
	__check_dist_euclidean( c, mat_col_gunpowder, particle_gunpowder );
	__check_dist_euclidean( c, mat_col_oil, particle_oil );
	__check_dist_euclidean( c, mat_col_lava, particle_lava );
	__check_dist_euclidean( c, mat_col_stone, particle_stone );
	__check_dist_euclidean( c, mat_col_acid, particle_acid );

	return p;
}

void drop_file_callback( void* platform_window, s32 count, const char** file_paths )
{
	if ( count < 1 ) return;

	// Just do first one for now...
	if ( count > 1 ) count = 1;

	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// We'll place at the mouse position as well, for shiggles
	gs_vec2 mp = calculate_mouse_position();

	for ( s32 i = 0; i < count; ++i )
	{
		// Need to verify this IS an image first.
		char temp_file_extension_buffer[ 16 ] = {0}; 
		gs_util_get_file_extension( temp_file_extension_buffer, sizeof( temp_file_extension_buffer ), file_paths[ 0 ] );
		if ( gs_string_compare_equal(temp_file_extension_buffer, "png" ) || 
			 gs_string_compare_equal(temp_file_extension_buffer, "jpg" ) || 
			 gs_string_compare_equal(temp_file_extension_buffer, "jpeg") || 
			 gs_string_compare_equal(temp_file_extension_buffer, "bmp" ) )
		{
			// Load texture into memory
			s32 _w, _h, _n;
			void* texture_data = NULL;

			// Force texture data to 3 components
			texture_data = gfx->load_texture_data_from_file( file_paths[ i ], false, gs_texture_format_rgb8, &_w, &_h, &_n );
			_n = 3;

			// Not sure what the format should be, so this is ...blah. Need to find a way to determine this beforehand.
			u8* data = (u8*)texture_data;

			s32 sx = ( g_texture_width - _w ) / 2;
			s32 sy = ( g_texture_height - _h ) / 2;

			// Now we need to process the data and place it into our particle/color buffers
			for ( u32 h = 0; h < _h; ++h ) 
			{
				for ( u32 w = 0; w < _w; ++w ) 
				{
					color_t c = 
					{
						data[ (h * _w + w) * _n + 0 ],
						data[ (h * _w + w) * _n + 1 ],
						data[ (h * _w + w) * _n + 2 ],
						255
					};

					// Get color of this pixel in the image
					particle_t p = get_closest_particle_from_color( c );

					// Let's place this thing in the center instead...
					if ( in_bounds( sx + w, sy + h ) ) {

						u32 idx = compute_idx( sx + w, sy + h );
						write_data( idx, p );
					}
				}
			}

			// Free texture data
			gs_free( texture_data );
		}

	}
}

// Here, we'll initialize all of our application data, which in this case is our graphics resources
gs_result app_init()
{
	// Cache instance of api contexts from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Construct command buffer ( the command buffer is used to allow for immediate drawing at any point in our program )
	g_cb = gfx->construct_command_buffer();

	// Construct shader from our source above
	g_shader = gfx->construct_shader( v_src, f_src );

	// Construct uniform for shader
	u_tex = gfx->construct_uniform( g_shader, "u_tex", gs_uniform_type_sampler2d );
	u_flip_y = gfx->construct_uniform( g_shader, "u_flip_y", gs_uniform_type_int );

	// Vertex data layout for our mesh (for this shader, it's a single float2 attribute for position)
	gs_vertex_attribute_type layout[] = 
	{
		gs_vertex_attribute_float2,
		gs_vertex_attribute_float2
	};
	// Count of our vertex attribute array
	u32 layout_count = sizeof( layout ) / sizeof( gs_vertex_attribute_type ); 

	// Vertex data for triangle
	f32 v_data[] = 
	{
		// Positions  UVs
		-1.0f, -1.0f,  0.0f, 0.0f,	// Top Left
		 1.0f, -1.0f,  1.0f, 0.0f,	// Top Right 
		-1.0f,  1.0f,  0.0f, 1.0f,  // Bottom Left
		 1.0f,  1.0f,  1.0f, 1.0f   // Bottom Right
	};

	u32 i_data[] = 
	{
		0, 2, 3,
		3, 1, 0
	};

	// Construct vertex buffer
	g_vbo = gfx->construct_vertex_buffer( layout, layout_count, v_data, sizeof(v_data) );
	// Construct index buffer
	g_ibo = gfx->construct_index_buffer( i_data, sizeof(i_data) );

	// Construct world data ( for now, it'll just be the size of the screen )
	g_world_particle_data = gs_malloc( g_texture_width * g_texture_height * sizeof(particle_t) );

	// Construct texture buffer data
	g_texture_buffer = gs_malloc( g_texture_width * g_texture_height * sizeof(color_t) );

	g_ui_buffer = gs_malloc( g_texture_width * g_texture_height * sizeof(color_t) );

	// Set buffers to 0
	memset( g_texture_buffer, 0, g_texture_width * g_texture_height * sizeof(color_t) );
	memset( g_world_particle_data, 0, g_texture_width * g_texture_height );
	memset( g_ui_buffer, 0, g_texture_width * g_texture_height );

	// Construct texture resource from GPU
	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.texture_format = gs_texture_format_rgba8;
	t_desc.mag_filter = gs_nearest;
	t_desc.min_filter = gs_nearest;
	t_desc.generate_mips = false;
	t_desc.width = g_texture_width;
	t_desc.height = g_texture_height;
	t_desc.num_comps = 4;
	t_desc.data = g_texture_buffer;

	g_tex = gfx->construct_texture( t_desc );

	// Construct texture resource from GPU
	t_desc = gs_texture_parameter_desc_default();
	t_desc.texture_format = gs_texture_format_rgba8;
	t_desc.mag_filter = gs_nearest;
	t_desc.min_filter = gs_nearest;
	t_desc.generate_mips = false;
	t_desc.width = g_texture_width;
	t_desc.height = g_texture_height;
	t_desc.num_comps = 4;
	t_desc.data = g_ui_buffer;

	g_tex_ui = gfx->construct_texture( t_desc );

	// Construct target for offscreen rendering
	t_desc.data = NULL;
	g_rt = gfx->construct_texture( t_desc );

	// Construct frame buffer
	g_fb = gfx->construct_frame_buffer( g_rt );

	// Cache window handle from platform
	g_window = gs_engine_instance()->ctx.platform->main_window();

	// Initialize render passes
	g_blur_pass = blur_pass_ctor();
	g_bright_pass = bright_filter_pass_ctor();
	g_composite_pass = composite_pass_ctor();

	// Load UI font texture data from file
	construct_font_data();

	// Set up callback for dropping them files, yo.
	platform->set_dropped_files_callback( platform->main_window(), &drop_file_callback );

	return gs_result_success;
}

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	// If we press the escape key, exit the application
	if ( engine->ctx.platform->key_pressed( gs_keycode_esc ) )
	{
		return gs_result_success;
	}

	// Why not print this elsewhere, yo?
	gs_timed_action( 60, {
		gs_println( "frame: %.5f ms", engine->ctx.platform->time.frame );
	});

	// All application updates
	b32 ui_interaction = update_ui();
	if ( !ui_interaction ) {
		update_input();
	}

	if ( g_run_simulation ) {
		update_particle_sim();
	}

	/*===============
	// Render scene
	================*/
	render_scene();

	// Update frame counter
	g_frame_counter = (g_frame_counter + 1) % u32_max;

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}

void construct_font_data()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	g_font.data = g_font_data;
	g_font.width = 90;
	g_font.height = 42;
	g_font.num_comps = 3;

	// Set up metrics
	g_font.glyph_advance = 1;

	// Construct glyph information
	const s32 glyph_width = 5, glyph_height = 7;
	for ( u32 r = 0; r < 6; ++r ) 
	{
		for ( u32 c = 0; c < 18; ++c ) 
		{
			u32 idx = r * 18 + c;
			g_font.glyphs[ idx ] = (font_glyph_t){ c * 5, r * 7, 5, 7 };
		}
	}
}

font_glyph_t get_glyph( font_t* f, char c )
{
	switch ( c ) 
	{
		case ' ': return g_font.glyphs[ 0 ];
		case '!': return g_font.glyphs[ 1 ];
		case '"': return g_font.glyphs[ 2 ];
		case '#': return g_font.glyphs[ 3 ];
		case '$': return g_font.glyphs[ 4 ];
		case '%': return g_font.glyphs[ 5 ];
		case '&': return g_font.glyphs[ 6 ];
		case '\'': return g_font.glyphs[ 7 ];
		case '(': return g_font.glyphs[ 8 ];
		case ')': return g_font.glyphs[ 9 ];
		case '*': return g_font.glyphs[ 10 ];
		case '+': return g_font.glyphs[ 11 ];
		case ',': return g_font.glyphs[ 12 ];
		case '-': return g_font.glyphs[ 13 ];
		case '.': return g_font.glyphs[ 14 ];
		case '/': return g_font.glyphs[ 15 ];
		case '0': return g_font.glyphs[ 16 ];
		case '1': return g_font.glyphs[ 17 ];
		case '2': return g_font.glyphs[ 18 ];
		case '3': return g_font.glyphs[ 19 ];
		case '4': return g_font.glyphs[ 20 ];
		case '5': return g_font.glyphs[ 21 ];
		case '6': return g_font.glyphs[ 22 ];
		case '7': return g_font.glyphs[ 23 ];
		case '8': return g_font.glyphs[ 24 ];
		case '9': return g_font.glyphs[ 25 ];
		case ':': return g_font.glyphs[ 26 ];
		case ';': return g_font.glyphs[ 27 ];
		case '<': return g_font.glyphs[ 28 ];
		case '=': return g_font.glyphs[ 29 ];
		case '>': return g_font.glyphs[ 30 ];
		case '?': return g_font.glyphs[ 31 ];
		case '@': return g_font.glyphs[ 32 ];
		case 'A': return g_font.glyphs[ 33 ];
		case 'B': return g_font.glyphs[ 34 ];
		case 'C': return g_font.glyphs[ 35 ];
		case 'D': return g_font.glyphs[ 36 ];
		case 'E': return g_font.glyphs[ 37 ];
		case 'F': return g_font.glyphs[ 38 ];
		case 'G': return g_font.glyphs[ 39 ];
		case 'H': return g_font.glyphs[ 40 ];
		case 'I': return g_font.glyphs[ 41 ];
		case 'J': return g_font.glyphs[ 42 ];
		case 'K': return g_font.glyphs[ 43 ];
		case 'L': return g_font.glyphs[ 44 ];
		case 'M': return g_font.glyphs[ 45 ];
		case 'N': return g_font.glyphs[ 46 ];
		case 'O': return g_font.glyphs[ 47 ];
		case 'P': return g_font.glyphs[ 48 ];
		case 'Q': return g_font.glyphs[ 49 ];
		case 'R': return g_font.glyphs[ 50 ];
		case 'S': return g_font.glyphs[ 51 ];
		case 'T': return g_font.glyphs[ 52 ];
		case 'U': return g_font.glyphs[ 53 ];
		case 'V': return g_font.glyphs[ 54 ];
		case 'W': return g_font.glyphs[ 55 ];
		case 'X': return g_font.glyphs[ 56 ];
		case 'Y': return g_font.glyphs[ 57 ];
		case 'Z': return g_font.glyphs[ 58 ];
		case '[': return g_font.glyphs[ 59 ];
		case '\\': return g_font.glyphs[ 60 ];
		case ']': return g_font.glyphs[ 61 ];
		case '^': return g_font.glyphs[ 62 ];
		case '_': return g_font.glyphs[ 63 ];
		case '`': return g_font.glyphs[ 64 ];
		case 'a': return g_font.glyphs[ 65 ];
		case 'b': return g_font.glyphs[ 66 ];
		case 'c': return g_font.glyphs[ 67 ];
		case 'd': return g_font.glyphs[ 68 ];
		case 'e': return g_font.glyphs[ 69 ];
		case 'f': return g_font.glyphs[ 70 ];
		case 'g': return g_font.glyphs[ 71 ];
		case 'h': return g_font.glyphs[ 72 ];
		case 'i': return g_font.glyphs[ 73 ];
		case 'j': return g_font.glyphs[ 74 ];
		case 'k': return g_font.glyphs[ 75 ];
		case 'l': return g_font.glyphs[ 76 ];
		case 'm': return g_font.glyphs[ 77 ];
		case 'n': return g_font.glyphs[ 78 ];
		case 'o': return g_font.glyphs[ 79 ];
		case 'p': return g_font.glyphs[ 80 ];
		case 'q': return g_font.glyphs[ 81 ];
		case 'r': return g_font.glyphs[ 82 ];
		case 's': return g_font.glyphs[ 83 ];
		case 't': return g_font.glyphs[ 84 ];
		case 'u': return g_font.glyphs[ 85 ];
		case 'v': return g_font.glyphs[ 86 ];
		case 'w': return g_font.glyphs[ 87 ];
		case 'x': return g_font.glyphs[ 88 ];
		case 'y': return g_font.glyphs[ 89 ];
		case 'z': return g_font.glyphs[ 90 ];
		case '{': return g_font.glyphs[ 91 ];
		case '|': return g_font.glyphs[ 92 ];
		case '}': return g_font.glyphs[ 93 ];
		case '~': return g_font.glyphs[ 94 ];

		// For anything not supported, just return empty space
		default: {
			return (font_glyph_t){0};
		} break;
	}
}

void putpixel( int x, int y ) {
	if ( in_bounds( x, y ) ) {
		g_ui_buffer[ compute_idx( x, y ) ] = (color_t){ 255, 255, 255, 255 };
	}
}

// Function to put pixels 
// at subsequence points 
void drawCircle(int xc, int yc, int x, int y) 
{ 
    putpixel(xc+x, yc+y); 
    putpixel(xc-x, yc+y); 
    putpixel(xc+x, yc-y); 
    putpixel(xc-x, yc-y); 
    putpixel(xc+y, yc+x); 
    putpixel(xc-y, yc+x); 
    putpixel(xc+y, yc-x); 
    putpixel(xc-y, yc-x); 
}

// Function for circle-generation 
// using Bresenham's algorithm 
void circleBres(int xc, int yc, int r) 
{ 
    int x = 0, y = r; 
    int d = 3 - 2 * r; 
    drawCircle(xc, yc, x, y); 
    while (y >= x) 
    { 
        // For each pixel we will 
        // draw all eight pixels
        x++; 
  
        // Check for decision parameter 
        // and correspondingly  
        // update d, x, y 
        if (d > 0) 
        { 
            y--;  
            d = d + 4 * (x - y) + 10; 
        } 
        else
            d = d + 4 * x + 6; 
        drawCircle(xc, yc, x, y); 
    } 
} 

void update_input()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	if ( platform->key_pressed( gs_keycode_i ) ) {
		g_show_material_selection_panel = !g_show_material_selection_panel;
	}

	if ( platform->key_pressed( gs_keycode_f ) ) {
		g_show_frame_count = !g_show_frame_count;
	}

	if ( platform->key_pressed( gs_keycode_b ) ) {
		g_use_post_processing = !g_use_post_processing;
	}

	f32 wx = 0, wy = 0;
	platform->mouse_wheel( &wx, &wy );
	if ( platform->key_pressed( gs_keycode_lbracket ) || wy < 0.f ) {
		g_selection_radius = gs_clamp( g_selection_radius - 1.f, 1.f, 100.f );
	}
	if ( platform->key_pressed( gs_keycode_rbracket ) || wy > 0.f ) {
		g_selection_radius = gs_clamp( g_selection_radius + 1.f, 1.f, 100.f );
	}

	if ( platform->key_pressed( gs_keycode_p ) ) {
		g_run_simulation = !g_run_simulation;
	}

	// Clear data
	if ( platform->key_pressed( gs_keycode_c ) ) {
		memset( g_texture_buffer, 0, sizeof(color_t) * g_texture_width * g_texture_height );
		memset( g_world_particle_data, 0, sizeof(particle_t) * g_texture_width * g_texture_height );
	}

	// Mouse input for testing
	if ( platform->mouse_down( gs_mouse_lbutton ) )
	{
		gs_vec2 mp = calculate_mouse_position();
		f32 mp_x = gs_clamp( mp.x, 0.f, (f32)g_texture_width - 1.f );	
		f32 mp_y = gs_clamp( mp.y, 0.f, (f32)g_texture_height - 1.f );
		u32 max_idx = (g_texture_width * g_texture_height) - 1;
		s32 r_amt = random_val( 1, 10000 );
		const f32 R = g_selection_radius;

		// Spawn in a circle around the mouse
		for ( u32 i = 0; i < r_amt; ++i )
		{
			f32 ran = (f32)random_val(0, 100) / 100.f;
			f32 r = R * sqrt(ran);
			f32 theta = (f32)random_val(0, 100)/100.f * 2.f * gs_pi;
			f32 rx = cos((f32)theta) * r;
			f32 ry = sin((f32)theta) * r;
			s32 mpx = (s32)gs_clamp( mp_x + (f32)rx, 0.f, (f32)g_texture_width - 1.f );
			s32 mpy = (s32)gs_clamp( mp_y + (f32)ry, 0.f, (f32)g_texture_height - 1.f );
			s32 idx = mpy * (s32)g_texture_width + mpx;
			idx = gs_clamp( idx, 0, max_idx );

			if ( is_empty( mpx, mpy ) )
			{
				particle_t p = {0};
				switch ( g_material_selection ) {
					case mat_sel_sand: p = particle_sand(); break;;
					case mat_sel_water: p = particle_water(); break;;
					case mat_sel_salt: p = particle_salt(); break;;
					case mat_sel_wood: p = particle_wood(); break;;
					case mat_sel_fire: p = particle_fire(); break;
					case mat_sel_smoke: p = particle_smoke(); break;
					case mat_sel_steam: p = particle_steam(); break;
					case mat_sel_gunpowder: p = particle_gunpowder(); break;
					case mat_sel_oil: p = particle_oil(); break;
					case mat_sel_lava: p = particle_lava(); break;
					case mat_sel_stone: p = particle_stone(); break;
					case mat_sel_acid: p = particle_acid(); break;
				}
				p.velocity = (gs_vec2){ random_val( -1, 1 ), random_val( -2, 5 ) };
				write_data( idx, p );
			}
		}
	}

	// Solid Erase
	if (  platform->mouse_down( gs_mouse_rbutton ) )
	{
		gs_vec2 mp = calculate_mouse_position( );
		f32 mp_x = gs_clamp( mp.x, 0.f, (f32)g_texture_width - 1.f );	
		f32 mp_y = gs_clamp( mp.y, 0.f, (f32)g_texture_height - 1.f );
		u32 max_idx = (g_texture_width * g_texture_height) - 1;
		const f32 R = g_selection_radius;

		// Erase in a circle pattern
		for ( s32 i = -R ; i < R; ++i )
		{
			for ( s32 j = -R ; j < R; ++j )
			{
				s32 rx = ((s32)mp_x + j); 
				s32 ry = ((s32)mp_y + i);
				gs_vec2 r = (gs_vec2){ rx, ry };

				if ( in_bounds( rx, ry ) && gs_vec2_dist( mp, r ) <= R ) {
					write_data( compute_idx( rx, ry ), particle_empty() );
				}
			}
		}
	}

	// Need to detect if mouse has entered the screen with payload...
}

void update_particle_sim()
{
	// Cache engine subsystem interfaces
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Update frame counter ( loop back to 0 if we roll past u32 max )
	b32 frame_counter_even = ((g_frame_counter % 2) == 0);
	s32 ran = frame_counter_even ? 0 : 1;

	const f32 dt = platform->time.delta;

	// Rip through read data and update write buffer
	// Note(John): We update "bottom up", since all the data is edited "in place". Double buffering all data would fix this 
	// 	issue, however it requires double all of the data.
	for ( u32 y = g_texture_height - 1; y > 0; --y )
	{
		for ( u32 x = ran ? 0 : g_texture_width - 1; ran ? x < g_texture_width : x > 0; ran ? ++x : --x )
		{
			// Current particle idx
			u32 read_idx = compute_idx( x, y );

			// Get material of particle at point
			u8 mat_id = get_particle_at( x, y ).id;

			// Update particle's lifetime (I guess just use frames)? Or should I have sublife?
			g_world_particle_data[ read_idx ].life_time += 1.f * dt;

			switch ( mat_id ) {

				case mat_id_sand:  update_sand( x, y );  break;
				case mat_id_water: update_water( x, y ); break;
				case mat_id_salt:  update_salt( x, y );  break;
				case mat_id_fire:  update_fire( x, y );  break;
				case mat_id_smoke: update_smoke( x, y ); break;
				case mat_id_ember: update_ember( x, y ); break;
				case mat_id_steam: update_steam( x, y ); break;
				case mat_id_gunpowder: update_gunpowder( x, y ); break;
				case mat_id_oil: update_oil( x, y ); break;
				case mat_id_lava: update_lava( x, y ); break;
				case mat_id_acid: update_acid( x, y ); break;

				// Do nothing for empty or default case
				default:
				case mat_id_empty: 
				{
					// update_default( w, h, i ); 
				} break;
			}
		}
	}

	// Can remove this loop later on by keeping update structure and setting that for the particle as it moves, 
	// then at the end of frame just memsetting the entire structure to 0.
	for ( u32 y = g_texture_height - 1; y > 0; --y ) {
		for ( u32 x = ran ? 0 : g_texture_width - 1; ran ? x < g_texture_width : x > 0; ran ? ++x : --x ) {
			// Set particle's update to false for next frame
			g_world_particle_data[ compute_idx( x, y ) ].has_been_updated_this_frame = false;
		}
	}
}

void draw_glyph_at( font_t* f, color_t* buffer, s32 x, s32 y, char c, color_t col ) 
{
	u8* font_data = (u8*)f->data;
	font_glyph_t g = get_glyph( f, c );

	// How to accurately place? I have width and height of glyph in texture, but need to convert this to RGBA data for ui buffer
	for ( s32 h = 0; h < g.height; ++h ) 
	{
		for ( s32 w = 0; w < g.width; ++w ) 
		{
			s32 _w = w + g.x;
			s32 _h = h + g.y;
			u8 a = font_data[ ( _h * f->width + _w ) * f->num_comps + 0 ] == 0 ? 0 : 255;
			color_t c = {
				font_data[ ( _h * f->width + _w ) * f->num_comps + 0 ],
				font_data[ ( _h * f->width + _w ) * f->num_comps + 1 ],
				font_data[ ( _h * f->width + _w ) * f->num_comps + 2 ],
				a
			};
			if ( in_bounds( x + w, y + h ) && a ) {
				buffer[ compute_idx( x + w, y + h ) ] = col;
			}
		}
	}	
}

void draw_string_at( font_t* f, color_t* buffer, s32 x, s32 y, const char* str, usize len, color_t col ) 
{
	u8* font_data = (u8*)f->data;
	for ( u32 i = 0; i < len; ++i )
	{
		font_glyph_t g = get_glyph( f, str[i] );
		draw_glyph_at( f, buffer, x, y, str[i], col );
		x += g.width + f->glyph_advance;	// Move by glyph width + advance
	}
}

b32 in_rect ( gs_vec2 p, gs_vec2 ro, gs_vec2 rd ) 
{
	if ( p.x < ro.x || p.x > ro.x + rd.x || p.y < ro.y || p.y > ro.y + rd.y ) return false;
	return true;
}

b32 gui_rect( color_t* buffer, s32 _x, s32 _y, s32 _w, s32 _h, color_t c ) 
{
	gs_vec2 mp = calculate_mouse_position();

	for ( u32 h = 0; h < _h; ++ h ) 
	{
		for ( u32 w = 0; w < _w; ++w )
		{
			if ( in_bounds( _x + w, _y + h ) ) {
				buffer[ compute_idx( _x + w, _y + h ) ] = c;	
			}
		}
	}

	b32 clicked = gs_engine_instance()->ctx.platform->mouse_pressed( gs_mouse_lbutton );

	return in_rect( mp, (gs_vec2){ _x, _y }, (gs_vec2){ _w, _h } ) && clicked ; 
}

#define __gui_interaction( x, y, w, h, c, str, id )\
do {\
	if ( (id) == g_material_selection ) {\
		const s32 b = 2;\
		gui_rect( g_ui_buffer, x - b / 2, y - b / 2, w + b, h + b, (color_t){ 200, 150, 20, 255 } );\
	}\
	gs_vec2 mp = calculate_mouse_position();\
	if ( in_rect( mp, (gs_vec2){ (x), (y) }, (gs_vec2){ (w), (h) })) {\
		interaction |= true;\
		char _str[] = (str);\
		color_t col = (color_t){ 255, 255, 255, 255 };\
		color_t s_col = (color_t){ 10, 10, 10, 255 };\
		color_t r_col = (color_t){ 5, 5, 5, 170 };\
		/*Draw rect around text as well for easier viewing*/\
		gui_rect(g_ui_buffer, g_texture_width / 2 - 50, 15, 100, 20, r_col);\
		draw_string_at(&g_font, g_ui_buffer, g_texture_width / 2 + 1 - (sizeof(str) * 5) / 2, 20 - 1, _str, sizeof(_str), s_col);\
		draw_string_at(&g_font, g_ui_buffer, g_texture_width / 2 - (sizeof(str) * 5) / 2, 20, _str, sizeof(_str), col);\
	}\
	if ( gui_rect( g_ui_buffer, x, y, w, h, c ) ) {\
		g_material_selection = id;\
	}\
} while ( 0 )

b32 update_ui()
{
	b32 interaction = false;
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Cache transformed mouse position
	gs_vec2 mp = calculate_mouse_position();

	// Do ui stuff
	memset( g_ui_buffer, 0, g_texture_width * g_texture_height * sizeof(color_t) );

	// Material selection panel gui
	if ( g_show_material_selection_panel ) 
	{
		const s32 offset = 12;
		s32 xoff = 20;
		s32 base = 10;

		// Sand Selection
		__gui_interaction(g_texture_width - xoff, base + offset * 0, 10, 10, mat_col_sand, "Sand", mat_sel_sand );
		__gui_interaction(g_texture_width - xoff, base + offset * 1, 10, 10, mat_col_water, "Water", mat_sel_water );
		__gui_interaction(g_texture_width - xoff, base + offset * 2, 10, 10, mat_col_smoke, "Smoke", mat_sel_smoke );
		__gui_interaction(g_texture_width - xoff, base + offset * 3, 10, 10, mat_col_fire, "Fire", mat_sel_fire );
		__gui_interaction(g_texture_width - xoff, base + offset * 4, 10, 10, mat_col_steam, "Steam", mat_sel_steam );
		__gui_interaction(g_texture_width - xoff, base + offset * 5, 10, 10, mat_col_oil, "Oil", mat_sel_oil );
		__gui_interaction(g_texture_width - xoff, base + offset * 6, 10, 10, mat_col_salt, "Salt", mat_sel_salt );
		__gui_interaction(g_texture_width - xoff, base + offset * 7, 10, 10, mat_col_wood, "Wood", mat_sel_wood );
		__gui_interaction(g_texture_width - xoff, base + offset * 8, 10, 10, mat_col_stone, "Stone", mat_sel_stone );
		__gui_interaction(g_texture_width - xoff, base + offset * 9, 10, 10, mat_col_lava, "Lava", mat_sel_lava );
		__gui_interaction(g_texture_width - xoff, base + offset * 10, 10, 10, mat_col_gunpowder, "GunPowder", mat_sel_gunpowder );
		__gui_interaction(g_texture_width - xoff, base + offset * 11, 10, 10, mat_col_acid, "Acid", mat_sel_acid );
	}

	if ( g_show_frame_count ) {

		char frame_time_str[256];
		gs_snprintf (frame_time_str, sizeof(frame_time_str), "frame: %.2f ms", platform->time.frame );
		draw_string_at( &g_font, g_ui_buffer, 10, 10, frame_time_str, strlen(frame_time_str), (color_t){ 255, 255, 255, 255 } ); 

		char sim_state_str[256];
		gs_snprintf (sim_state_str, sizeof(sim_state_str), "state: %s", g_run_simulation ? "running" : "paused" );
		draw_string_at( &g_font, g_ui_buffer, 10, 20, sim_state_str, strlen(sim_state_str), (color_t){ 255, 255, 255, 255 } );
	}

	// Draw circle around mouse pointer
	s32 R = g_selection_radius;
	circleBres((s32)mp.x, (s32)mp.y, R); 

	// Upload our updated texture data to GPU
	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.mag_filter = gs_nearest;
	t_desc.min_filter = gs_nearest;
	t_desc.generate_mips = false;
	t_desc.width = g_texture_width;
	t_desc.height = g_texture_height;
	t_desc.num_comps = 4;
	t_desc.data = g_ui_buffer;
	gfx->update_texture_data( g_tex_ui, t_desc )	;

	return interaction;
}

void render_scene()
{
	// Graphics api instance
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Platform api instance
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Upload our updated texture data to GPU
	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.mag_filter = gs_nearest;
	t_desc.min_filter = gs_nearest;
	t_desc.generate_mips = false;
	t_desc.width = g_texture_width;
	t_desc.height = g_texture_height;
	t_desc.num_comps = 4;
	t_desc.data = g_texture_buffer;
	gfx->update_texture_data( g_tex, t_desc );

	gs_vec2 ws = platform->window_size( g_window );
	b32 flip_y = false;

	// Bind our render target and render offscreen
	gfx->bind_frame_buffer( g_cb, g_fb );
	{
		// Bind frame buffer attachment for rendering
		gfx->set_frame_buffer_attachment( g_cb, g_rt, 0 );

		// Set clear color and clear screen
		f32 clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.f };
		gfx->set_view_clear( g_cb, clear_color );

		// This is to handle mac's retina high dpi for now until I fix that internally.
		gfx->set_view_port( g_cb, g_texture_width, g_texture_height );
		gfx->bind_shader( g_cb, g_shader );
		gfx->bind_uniform( g_cb, u_flip_y, &flip_y );
		gfx->bind_vertex_buffer( g_cb, g_vbo );
		gfx->bind_index_buffer( g_cb, g_ibo );
		gfx->bind_texture( g_cb, u_tex, g_tex, 0 );
		gfx->draw_indexed( g_cb, 6 );
	}
	// Unbind offscreen buffer
	gfx->unbind_frame_buffer( g_cb );

	// Bind frame buffer for post processing
	gfx->bind_frame_buffer( g_cb, g_fb );
	{
		// Brightness pass
		{
			bright_filter_pass_parameters_t params = (bright_filter_pass_parameters_t){ g_rt };
			render_pass_i* p = gs_cast( render_pass_i, &g_bright_pass );
			p->pass( g_cb, p, &params );
		}

		// Blur pass
		{
			blur_pass_parameters_t params = (blur_pass_parameters_t){ g_bright_pass.data.render_target };
			render_pass_i* p = gs_cast( render_pass_i, &g_blur_pass );
			p->pass( g_cb, p, &params );
		}

		// composite pass w/ gamma correction
		{
			composite_pass_parameters_t params = (composite_pass_parameters_t){ g_rt, g_blur_pass.data.blur_render_target_b };
			render_pass_i* p = gs_cast( render_pass_i, &g_composite_pass );
			p->pass( g_cb, p, &params );
		}
	}
	gfx->unbind_frame_buffer( g_cb );

	// Back buffer Presentation
	{
		// Set clear color and clear screen
		f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
		gfx->set_view_clear( g_cb, clear_color );

		// This is to handle mac's retina high dpi for now until I fix that internally.
	#if (defined GS_PLATFORM_APPLE)
		gfx->set_view_port( g_cb, (s32)ws.x * 2, (s32)ws.y * 2 );
	#else
		gfx->set_view_port( g_cb, (s32)ws.x, (s32)ws.y );
	#endif

		f32 t = gs_engine_instance()->ctx.platform->elapsed_time() * gs_engine_instance()->ctx.platform->time.delta * 0.001f;
		flip_y = true;

		gfx->bind_shader( g_cb, g_shader );
		gfx->bind_uniform( g_cb, u_flip_y, &flip_y );		
		gfx->bind_vertex_buffer( g_cb, g_vbo );
		gfx->bind_index_buffer( g_cb, g_ibo );

		// Draw final composited image
		if (g_use_post_processing) {

			gfx->bind_texture( g_cb, u_tex, g_composite_pass.data.render_target, 0 );
		} else {

			gfx->bind_texture( g_cb, u_tex, g_rt, 0 );
		}
		gfx->draw_indexed( g_cb, 6 );

		// Draw UI on top
		gfx->bind_texture( g_cb, u_tex, g_tex_ui, 0 );
		gfx->draw_indexed( g_cb, 6 );
	}

	// Submit command buffer for rendering
	gfx->submit_command_buffer( g_cb );
}

void write_data( u32 idx, particle_t p )
{
	// Write into particle data for id value
	g_world_particle_data[ idx ] = p;
	g_texture_buffer[ idx ] = p.color;
}

void update_salt( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 2;
	u32 spread_rate = 5;
	s32 lx, ly;

	p->velocity.y = gs_clamp( p->velocity.y + (gravity * dt), -10.f, 10.f );

	p->has_been_updated_this_frame = true;

	// If in liquid, chance to dissolve itself.
	if ( is_in_liquid( x, y, &lx, &ly ) ) {
		if ( random_val( 0, 1000 ) == 0 ) {
			write_data( read_idx, particle_empty() );
			return;
		}
	}

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	// if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) && get_particle_at( x, y + 1 ).id != mat_id_water ) {
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) ) {
		p->velocity.y /= 2.f;
	}

	s32 r = 1;
	s32 l = -r;
	s32 u = fall_rate;
	s32 v_idx = compute_idx ( x + (s32)p->velocity.x, y + (s32)p->velocity.y );
	s32 b_idx = compute_idx( x, y + u );
	s32 bl_idx = compute_idx( x + l, y + u );
	s32 br_idx = compute_idx( x + r, y + u );
	s32 l_idx = compute_idx( x + l, y );
	s32 r_idx = compute_idx( x + r, y );
	s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;

	if ( in_bounds( x + vx, y + vy ) && (is_empty( x + vx, y + vy ) ) ) {
		write_data( v_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_in_liquid( x, y, &lx, &ly ) && random_val( 0, 10 ) == 0 ) {
		particle_t tmp_b = get_particle_at( lx, ly );
		write_data( compute_idx( lx, ly ), *p );
		write_data( read_idx, tmp_b );
	}
// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y + 1 ) && (( is_empty( x, y + 1 ) ) ) ) {
		u32 idx = compute_idx( x, y + 1 );
		p->velocity.y += (gravity * dt );
		particle_t tmp_a = g_world_particle_data[ read_idx ];
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, tmp_a );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y + 1 ) && ( is_empty( x - 1, y + 1 ) ) ) {
		u32 idx = compute_idx( x - 1, y + 1 );
		p->velocity.x = random_val( 0, 1 ) == 0 ? -1.2f : 1.2f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_a = g_world_particle_data[ read_idx ];
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, tmp_a );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + 1, y + 1 ) && ( is_empty( x + 1, y + 1 )) ) {
		u32 idx = compute_idx( x + 1, y + 1 );
		p->velocity.x = random_val( 0, 1 ) == 0 ? -1.2f : 1.2f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_a = g_world_particle_data[ read_idx ];
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, tmp_a );
		write_data( read_idx, tmp_b );
	}
}

void update_sand( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 4;

	p->velocity.y = gs_clamp( p->velocity.y + (gravity * dt), -10.f, 10.f );

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) && get_particle_at( x, y + 1 ).id != mat_id_water ) {
		p->velocity.y /= 2.f;
		}

	s32 vi_x = x + (s32)p->velocity.x; 
	s32 vi_y = y + (s32)p->velocity.y;

	// Check to see if you can swap first with other element below you
	u32 b_idx = compute_idx( x, y + 1 );
	u32 br_idx = compute_idx( x + 1, y + 1 );
	u32 bl_idx = compute_idx( x - 1, y + 1 );

	s32 lx, ly;

	particle_t tmp_a = g_world_particle_data[ read_idx ];

	// Physics (using velocity)
	if ( in_bounds( vi_x, vi_y ) && (( is_empty( vi_x, vi_y ) ||
			((( g_world_particle_data[ compute_idx( vi_x, vi_y ) ].id == mat_id_water ) && 
			  !g_world_particle_data[ compute_idx( vi_x, vi_y ) ].has_been_updated_this_frame && 
			   gs_vec2_len(g_world_particle_data[compute_idx(vi_x, vi_y)].velocity) - gs_vec2_len(tmp_a.velocity) > 10.f ) ) ) ) ) {

		particle_t tmp_b = g_world_particle_data[ compute_idx( vi_x, vi_y ) ];

		// Try to throw water out
		if ( tmp_b.id == mat_id_water ) {

			s32 rx = random_val( -2, 2 );
			tmp_b.velocity = (gs_vec2){ rx, -4.f };

			write_data( compute_idx( vi_x, vi_y ), tmp_a );	

			for( s32 i = -10; i < 0; ++i ) {
				for ( s32 j = -10; j < 10; ++j ) {
					if ( is_empty( vi_x + j, vi_y + i ) ) {
						write_data( compute_idx( vi_x + j, vi_y + i ), tmp_b );
						break;
					}	
				}
			}

			// Couldn't write there, so, uh, destroy it.
			write_data( read_idx, particle_empty() );
		}
		else if ( is_empty( vi_x, vi_y ) ) {
			write_data( compute_idx( vi_x, vi_y ), tmp_a );
			write_data( read_idx, tmp_b );
		}
	}
	// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y + 1 ) && (( is_empty( x, y + 1 ) || ( g_world_particle_data[ b_idx ].id == mat_id_water ) ) ) ) {
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x, y + 1 );
		write_data( b_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y + 1 ) && (( is_empty( x - 1, y + 1 ) || g_world_particle_data[ bl_idx ].id == mat_id_water )) ) {
		p->velocity.x = is_in_liquid( x, y, &lx, &ly ) ? 0.f : random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x - 1, y + 1 );
		write_data( bl_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + 1, y + 1 ) && (( is_empty( x + 1, y + 1 ) || g_world_particle_data[ br_idx ].id == mat_id_water )) ) {
		p->velocity.x = is_in_liquid( x, y, &lx, &ly ) ? 0.f : random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x + 1, y + 1 );
		write_data( br_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( is_in_liquid( x, y, &lx, &ly ) && random_val( 0, 10 ) == 0 ) {
		particle_t tmp_b = get_particle_at( lx, ly );
		write_data( compute_idx( lx, ly ), *p );
		write_data( read_idx, tmp_b );
	}
	
}

void update_gunpowder( u32 x, u32 y ) 
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 4;

	p->velocity.y = gs_clamp( p->velocity.y + (gravity * dt), -10.f, 10.f );
	// p->velocity.x = gs_clamp( p->velocity.x, -5.f, 5.f );

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) && get_particle_at( x, y + 1 ).id != mat_id_water ) {
		p->velocity.y /= 2.f;
		// p->velocity.x /= 1.2f;
	}

	s32 vi_x = x + (s32)p->velocity.x; 
	s32 vi_y = y + (s32)p->velocity.y;

	// Check to see if you can swap first with other element below you
	u32 b_idx = compute_idx( x, y + 1 );
	u32 br_idx = compute_idx( x + 1, y + 1 );
	u32 bl_idx = compute_idx( x - 1, y + 1 );

	s32 lx, ly;

	particle_t tmp_a = g_world_particle_data[ read_idx ];

	// Physics (using velocity)
	if ( in_bounds( vi_x, vi_y ) && (( is_empty( vi_x, vi_y ) ||
			((( g_world_particle_data[ compute_idx( vi_x, vi_y ) ].id == mat_id_water ) && 
			  !g_world_particle_data[ compute_idx( vi_x, vi_y ) ].has_been_updated_this_frame && 
			   gs_vec2_len(g_world_particle_data[compute_idx(vi_x, vi_y)].velocity) - gs_vec2_len(tmp_a.velocity) > 10.f ) ) ) ) ) {

		particle_t tmp_b = g_world_particle_data[ compute_idx( vi_x, vi_y ) ];

		// Try to throw water out
		if ( tmp_b.id == mat_id_water ) {

			s32 rx = random_val( -2, 2 );
			tmp_b.velocity = (gs_vec2){ rx, -4.f };

			write_data( compute_idx( vi_x, vi_y ), tmp_a );	

			for( s32 i = -10; i < 0; ++i ) {
				for ( s32 j = -10; j < 10; ++j ) {
					if ( is_empty( vi_x + j, vi_y + i ) ) {
						write_data( compute_idx( vi_x + j, vi_y + i ), tmp_b );
						break;
					}	
				}
			}

			// Couldn't write there, so, uh, destroy it.
			write_data( read_idx, particle_empty() );
		}
		else if ( is_empty( vi_x, vi_y ) ) {
			write_data( compute_idx( vi_x, vi_y ), tmp_a );
			write_data( read_idx, tmp_b );
		}
	}
	// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y + 1 ) && (( is_empty( x, y + 1 ) || ( g_world_particle_data[ b_idx ].id == mat_id_water ) ) ) ) {
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x, y + 1 );
		write_data( b_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y + 1 ) && (( is_empty( x - 1, y + 1 ) || g_world_particle_data[ bl_idx ].id == mat_id_water )) ) {
		p->velocity.x = is_in_liquid( x, y, &lx, &ly ) ? 0.f : random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x - 1, y + 1 );
		write_data( bl_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + 1, y + 1 ) && (( is_empty( x + 1, y + 1 ) || g_world_particle_data[ br_idx ].id == mat_id_water )) ) {
		p->velocity.x = is_in_liquid( x, y, &lx, &ly ) ? 0.f : random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x + 1, y + 1 );
		write_data( br_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( is_in_liquid( x, y, &lx, &ly ) && random_val( 0, 10 ) == 0 ) {
		particle_t tmp_b = get_particle_at( lx, ly );
		write_data( compute_idx( lx, ly ), *p );
		write_data( read_idx, tmp_b );
	}
}

void update_steam( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 4;

	if ( p->life_time > 10.f ) {
		write_data( read_idx, particle_empty() );
		return;
	}

	if ( p->has_been_updated_this_frame ) {
		return;
	}

	p->has_been_updated_this_frame = true;

	// Smoke rises over time. This might cause issues, actually...
	p->velocity.y = gs_clamp( p->velocity.y - (gravity * dt ), -2.f, 10.f );
	p->velocity.x = gs_clamp( p->velocity.x + (f32)random_val( -100, 100 ) / 100.f, -1.f, 1.f );

	// Change color based on life_time
	p->color.r = (u8)( gs_clamp( (gs_interp_linear( 0.f, 10.f, p->life_time ) / 10.f) * 255.f, 150.f, 255.f ) );
	p->color.g = (u8)( gs_clamp( (gs_interp_linear( 0.f, 10.f, p->life_time ) / 10.f) * 255.f, 150.f, 255.f ) );
	p->color.b = (u8)( gs_clamp( (gs_interp_linear( 0.f, 10.f, p->life_time ) / 10.f) * 255.f, 150.f, 255.f ) );
	p->color.a = (u8)( gs_clamp( (gs_interp_linear( 10.f, 0.f, p->life_time ) / 10.f) * 255.f, 10.f, 255.f ) );

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	if ( in_bounds( x, y - 1 ) && !is_empty( x, y - 1 ) && get_particle_at( x, y - 1 ).id != mat_id_water ) {
		p->velocity.y /= 2.f;
	}

	s32 vi_x = x + (s32)p->velocity.x; 
	s32 vi_y = y + (s32)p->velocity.y;

	if ( in_bounds( vi_x, vi_y ) && ( (is_empty( vi_x, vi_y ) || get_particle_at( vi_x, vi_y ).id == mat_id_water || get_particle_at( vi_x, vi_y ).id == mat_id_fire ) ) ) {

		particle_t tmp_b = g_world_particle_data[ compute_idx( vi_x, vi_y ) ];

		// Try to throw water out
		if ( tmp_b.id == mat_id_water ) {

			tmp_b.has_been_updated_this_frame = true;

			s32 rx = random_val( -2, 2 );
			tmp_b.velocity = (gs_vec2){ rx, -3.f };

			write_data( compute_idx( vi_x, vi_y ), *p );
			write_data( read_idx, tmp_b );

		}
		else if ( is_empty( vi_x, vi_y ) ) {
			write_data( compute_idx( vi_x, vi_y ), *p );
			write_data( read_idx, tmp_b );
		}
	}
	// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y - 1 ) && (( is_empty( x, y - 1 ) || ( get_particle_at( x, y - 1 ).id == mat_id_water ) || get_particle_at(x,y-1).id == mat_id_fire ) ) ) {
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x, y - 1 );
		write_data( compute_idx( x, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y - 1 ) && (( is_empty( x - 1, y - 1 ) || get_particle_at( x - 1, y - 1 ).id == mat_id_water ) || get_particle_at(x-1, y-1).id == mat_id_fire) ) {
		p->velocity.x = random_val( 0, 1 ) == 0 ? -1.2f : 1.2f;
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x - 1, y - 1 );
		write_data( compute_idx( x - 1, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + 1, y - 1 ) && (( is_empty( x + 1, y - 1 ) || get_particle_at( x + 1, y - 1 ).id == mat_id_water ) || get_particle_at(x+1, y-1).id == mat_id_fire) ) {
		p->velocity.x = random_val( 0, 1 ) == 0 ? -1.2f : 1.2f;
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x + 1, y - 1 );
		write_data( compute_idx( x + 1, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	// Can move if in liquid
	else if ( in_bounds( x + 1, y ) && ( get_particle_at( x + 1, y ).id == mat_id_water  ) ) {
		u32 idx = compute_idx( x + 1, y );
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y ) && ( g_world_particle_data[ compute_idx( x - 1, y ) ].id == mat_id_water ) ) {
		u32 idx = compute_idx( x - 1, y );
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else {
		write_data( read_idx, *p );
	}
}

void update_smoke( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 4;

	if ( p->life_time > 10.f ) {
		write_data( read_idx, particle_empty() );
		return;
	}

	if ( p->has_been_updated_this_frame ) {
		return;
	}

	p->has_been_updated_this_frame = true;

	// Smoke rises over time. This might cause issues, actually...
	p->velocity.y = gs_clamp( p->velocity.y - (gravity * dt ), -2.f, 10.f );
	p->velocity.x = gs_clamp( p->velocity.x + (f32)random_val( -100, 100 ) / 100.f, -1.f, 1.f );

	// Change color based on life_time
	p->color.r = (u8)( gs_clamp( (gs_interp_linear( 10.f, 0.f, p->life_time * 0.5f ) / 10.f) * 150.f, 0.f, 150.f ) );
	p->color.g = (u8)( gs_clamp( (gs_interp_linear( 10.f, 0.f, p->life_time * 0.5f ) / 10.f) * 120.f, 0.f, 120.f ) );
	p->color.b = (u8)( gs_clamp( (gs_interp_linear( 10.f, 0.f, p->life_time * 0.5f ) / 10.f) * 100.f, 0.f, 100.f ) );

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	if ( in_bounds( x, y - 1 ) && !is_empty( x, y - 1 ) && get_particle_at( x, y - 1 ).id != mat_id_water ) {
		p->velocity.y /= 2.f;
	}

	s32 vi_x = x + (s32)p->velocity.x; 
	s32 vi_y = y + (s32)p->velocity.y;

	// if ( in_bounds( vi_x, vi_y ) && ( (is_empty( vi_x, vi_y ) || get_particle_at( vi_x, vi_y ).id == mat_id_water || get_particle_at( vi_x, vi_y ).id == mat_id_fire ) ) ) {
	if ( in_bounds( vi_x, vi_y ) && get_particle_at( vi_x, vi_y ).id != mat_id_smoke ) {

		particle_t tmp_b = g_world_particle_data[ compute_idx( vi_x, vi_y ) ];

		// Try to throw water out
		if ( tmp_b.id == mat_id_water ) {

			tmp_b.has_been_updated_this_frame = true;

			s32 rx = random_val( -2, 2 );
			tmp_b.velocity = (gs_vec2){ rx, -3.f };

			write_data( compute_idx( vi_x, vi_y ), *p );
			write_data( read_idx, tmp_b );

		}
		else if ( is_empty( vi_x, vi_y ) ) {
			write_data( compute_idx( vi_x, vi_y ), *p );
			write_data( read_idx, tmp_b );
		}
	}
	// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y - 1 ) && get_particle_at( x, y - 1 ).id != mat_id_smoke && 
									   get_particle_at( x, y - 1 ).id != mat_id_wood &&
									   get_particle_at( x, y - 1 ).id != mat_id_stone ) {
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x, y - 1 );
		write_data( compute_idx( x, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y - 1 ) && get_particle_at( x - 1, y - 1 ).id != mat_id_smoke && 
											get_particle_at( x - 1, y - 1 ).id != mat_id_wood && 
											get_particle_at( x - 1, y - 1 ).id != mat_id_stone ) {
		p->velocity.x = random_val( 0, 1 ) == 0 ? -1.2f : 1.2f;
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x - 1, y - 1 );
		write_data( compute_idx( x - 1, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + 1, y - 1 ) && get_particle_at( x + 1, y - 1 ).id != mat_id_smoke && 
											get_particle_at( x + 1, y - 1 ).id != mat_id_wood && 
											get_particle_at( x + 1, y - 1 ).id != mat_id_stone ) {
		p->velocity.x = random_val( 0, 1 ) == 0 ? -1.2f : 1.2f;
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x + 1, y - 1 );
		write_data( compute_idx( x + 1, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	// Can move if in liquid
	else if ( in_bounds( x + 1, y ) && get_particle_at( x + 1, y ).id != mat_id_smoke && 
										get_particle_at( x + 1, y ).id != mat_id_wood && 
										get_particle_at( x + 1, y ).id != mat_id_stone ) {
		u32 idx = compute_idx( x + 1, y );
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y ) && get_particle_at( x - 1, y ).id != mat_id_smoke && 
									get_particle_at( x - 1, y ).id != mat_id_wood && 
									get_particle_at( x - 1, y ).id != mat_id_stone ) {
		u32 idx = compute_idx( x - 1, y );
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else {
		write_data( read_idx, *p );
	}
}

void update_ember( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 4;

	if ( p->life_time > 0.5f ) {
		write_data( read_idx, particle_empty() );
		return;
	}

	if ( p->has_been_updated_this_frame ) {
		return;
	}

	p->has_been_updated_this_frame = true;

	p->velocity.y = gs_clamp( p->velocity.y - (gravity * dt ), -2.f, 10.f );
	p->velocity.x = gs_clamp( p->velocity.x + (f32)random_val( -100, 100 ) / 100.f, -1.f, 1.f );

	// If directly on top of some wall, then replace it
	if ( in_bounds( x, y + 1 ) && get_particle_at( x, y + 1 ).id == mat_id_wood && random_val( 0, 200 ) == 0 ) {
		write_data( compute_idx( x, y + 1 ), particle_fire() );	
	}
	else if ( in_bounds( x + 1, y + 1 ) && get_particle_at( x + 1, y + 1 ).id == mat_id_wood && random_val( 0, 200 ) == 0 ) {
		write_data( compute_idx( x + 1, y + 1 ), particle_fire() );	
	}
	else if ( in_bounds( x - 1, y + 1 ) && get_particle_at( x - 1, y + 1 ).id == mat_id_wood && random_val( 0, 200 ) == 0 ) {
		write_data( compute_idx( x - 1, y + 1 ), particle_fire() );	
	}
	else if ( in_bounds( x - 1, y ) && get_particle_at( x - 1, y ).id == mat_id_wood && random_val( 0, 200 ) == 0 ) {
		write_data( compute_idx( x - 1, y ), particle_fire() );	
	}
	else if ( in_bounds( x + 1, y ) && get_particle_at( x + 1, y ).id == mat_id_wood && random_val( 0, 200 ) == 0 ) {
		write_data( compute_idx( x + 1, y ), particle_fire() );	
	}
	else if ( in_bounds( x + 1, y - 1 ) && get_particle_at( x + 1, y - 1 ).id == mat_id_wood && random_val( 0, 200 ) == 0 ) {
		write_data( compute_idx( x + 1, y - 1 ), particle_fire() );	
	}
	else if ( in_bounds( x - 1, y - 1 ) && get_particle_at( x - 1, y - 1 ).id == mat_id_wood && random_val( 0, 200 ) == 0 ) {
		write_data( compute_idx( x - 1, y - 1 ), particle_fire() );	
	}

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	if ( in_bounds( x, y - 1 ) && !is_empty( x, y - 1 ) && get_particle_at( x, y - 1 ).id != mat_id_water ) {
		p->velocity.y /= 2.f;
	}

	s32 vi_x = x + (s32)p->velocity.x; 
	s32 vi_y = y + (s32)p->velocity.y;

	if ( in_bounds( vi_x, vi_y ) && ( is_empty( vi_x, vi_y ) || 
			get_particle_at( vi_x, vi_y ).id == mat_id_water ||
			get_particle_at( vi_x, vi_y ).id == mat_id_fire || 
			get_particle_at( vi_x, vi_y ).id == mat_id_smoke || 
			get_particle_at( vi_x, vi_y ).id == mat_id_ember ) ) {

		particle_t tmp_b = g_world_particle_data[ compute_idx( vi_x, vi_y ) ];

		// Try to throw water out
		if ( tmp_b.id == mat_id_water ) {

			tmp_b.has_been_updated_this_frame = true;

			s32 rx = random_val( -2, 2 );
			tmp_b.velocity = (gs_vec2){ rx, -3.f };

			write_data( compute_idx( vi_x, vi_y ), *p );
			write_data( read_idx, tmp_b );

		}
		else if ( is_empty( vi_x, vi_y ) ) {
			write_data( compute_idx( vi_x, vi_y ), *p );
			write_data( read_idx, tmp_b );
		}
	}
	// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y - 1 ) && (( is_empty( x, y - 1 ) || ( get_particle_at( x, y - 1 ).id == mat_id_water ) || get_particle_at(x,y-1).id == mat_id_fire ) ) ) {
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x, y - 1 );
		write_data( compute_idx( x, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y - 1 ) && (( is_empty( x - 1, y - 1 ) || get_particle_at( x - 1, y - 1 ).id == mat_id_water ) || get_particle_at(x-1, y-1).id == mat_id_fire) ) {
		p->velocity.x = random_val( 0, 1 ) == 0 ? -1.2f : 1.2f;
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x - 1, y - 1 );
		write_data( compute_idx( x - 1, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + 1, y - 1 ) && (( is_empty( x + 1, y - 1 ) || get_particle_at( x + 1, y - 1 ).id == mat_id_water ) || get_particle_at(x+1, y+1).id == mat_id_fire) ) {
		p->velocity.x = random_val( 0, 1 ) == 0 ? -1.2f : 1.2f;
		p->velocity.y -= (gravity * dt );
		particle_t tmp_b = get_particle_at( x + 1, y - 1 );
		write_data( compute_idx( x + 1, y - 1 ), *p );
		write_data( read_idx, tmp_b );
	}
	// Can move if in liquid
	else if ( in_bounds( x + 1, y ) && ( is_empty( x + 1, y ) || get_particle_at( x + 1, y ).id == mat_id_fire  ) ) {
		u32 idx = compute_idx( x + 1, y );
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y ) && ( is_empty( x - 1, y ) || get_particle_at( x - 1, y ).id == mat_id_fire ) ) {
		u32 idx = compute_idx( x - 1, y );
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else {
		write_data( read_idx, *p );
	}
}

void update_fire( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 4;

	if ( p->has_been_updated_this_frame ) {
		return;
	}

	p->has_been_updated_this_frame = true;

	if ( p->life_time > 0.2f ) {
		if ( random_val( 0, 100 ) == 0 ) {
			write_data( read_idx, particle_empty() );
			return;
		}
	}

	f32 st = sin(gs_engine_instance()->ctx.platform->elapsed_time());
	// f32 grav_mul = random_val( 0, 10 ) == 0 ? 2.f : 1.f;
	p->velocity.y = gs_clamp( p->velocity.y - ((gravity * dt)) * 0.2f , -5.0f, 0.f );
	// p->velocity.x = gs_clamp( st, -1.f, 1.f );
	p->velocity.x = gs_clamp( p->velocity.x + (f32)random_val( -100, 100 ) / 200.f, -0.5f, 0.5f );

	// Change color based on life_time

	if ( random_val( 0, (s32)(p->life_time * 100.f) ) % 200 == 0 ) {
		s32 ran = random_val( 0, 3 );
		switch ( ran ) {
			case 0: p->color = (color_t){ 255, 80, 20, 255 }; break;
			case 1: p->color = (color_t){ 250, 150, 10, 255 }; break;
			case 2: p->color = (color_t){ 200, 150, 0, 255 }; break;
			case 3: p->color = (color_t){ 100, 50, 2, 255 }; break;
		}
	}

	if ( p->life_time < 0.02f ) {
		p->color.r = 200;
	} else {
		p->color.r = 255;
	}

	// In water, so create steam and DIE
	// Should also kill the water...
	s32 lx, ly;
	if ( is_in_water( x, y, &lx, &ly ) ) {
		if ( random_val( 0, 1 ) == 0 ) {
			s32 ry = random_val( -5, -1 );
			s32 rx = random_val( -5, 5 );
			for ( s32 i = ry; i > -5; --i ) {
				for ( s32 j = rx; j < 5; ++j ) {
					particle_t p = particle_steam();
					if ( in_bounds( x + j, y + i ) && is_empty( x + j, y + i ) ) {
						particle_t p = particle_steam();
						write_data( compute_idx( x + j, y + i ), p );
					}
				}
			}
			particle_t p = particle_steam();
			write_data( read_idx, particle_empty() );
			write_data( read_idx, p );
			write_data( compute_idx( lx, ly ), particle_empty() );
			return;
		}
	}

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) && ( get_particle_at( x, y + 1 ).id != mat_id_water || get_particle_at( x, y + 1 ).id != mat_id_smoke ) ) {
		p->velocity.y /= 2.f;
	}

	if ( random_val( 0, 10 ) == 0 ) {
		// p->velocity.x = gs_clamp( p->velocity.x + (f32)random_val( -1, 1 ) / 2.f, -1.f, 1.f );
	}
	// p->velocity.x = gs_clamp( p->velocity.x, -0.5f, 0.5f );

	// Kill fire underneath
	if ( in_bounds( x, y + 3 ) && get_particle_at(x, y + 3).id == mat_id_fire && random_val(0, 100) == 0 ) {
		write_data( compute_idx(x, y +3 ), *p);
		write_data( read_idx, particle_empty() );
		return;
	}

	// Chance to kick itself up ( to simulate flames )
	if ( in_bounds( x, y + 1 ) && get_particle_at( x, y + 1 ).id == mat_id_fire && 
		in_bounds( x, y - 1 ) && get_particle_at( x, y - 1 ).id == mat_id_empty ) {
		if ( random_val( 0, 10 ) == 0 * p->life_time < 10.f && p->life_time > 1.f ) {
			s32 r = random_val(0, 1);
			s32 rh = random_val(-10, -1);
			s32 spread = 3;
			for ( s32 i = rh; i < 0; ++i ) {
				for ( s32 j = r ? -spread : spread; r ? j < spread : j > -spread; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						write_data( compute_idx( x + rx, y + ry ), *p );
						write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
		return;
	}

	s32 vi_x = x + (s32)p->velocity.x; 
	s32 vi_y = y + (s32)p->velocity.y;

	// Check to see if you can swap first with other element below you
	u32 b_idx = compute_idx( x, y + 1 );
	u32 br_idx = compute_idx( x + 1, y + 1 );
	u32 bl_idx = compute_idx( x - 1, y + 1 );

	const s32 wood_chance = 100;
	const s32 gun_powder_chance = 1;
	const s32 oil_chance = 5;

	// Chance to spawn smoke above
	for ( u32 i = 0; i < random_val( 1, 10 ); ++i ) {
		if ( random_val( 0, 500 ) == 0 ) {
			if ( in_bounds( x, y - 1 ) && is_empty( x, y - 1 ) ) {
				write_data( compute_idx( x, y - 1 ), particle_smoke() );
			}
			else if ( in_bounds( x + 1, y - 1 ) && is_empty( x + 1, y - 1 ) ) {
				write_data( compute_idx( x + 1, y - 1 ), particle_smoke() );
			}
			else if ( in_bounds( x - 1, y - 1 ) && is_empty( x - 1, y - 1 ) ) {
				write_data( compute_idx( x - 1, y - 1 ), particle_smoke() );
			}
		}
	}

	// Spawn embers
	if ( random_val( 0, 250 ) == 0 && p->life_time < 3.f ) {
		for ( u32 i = 0; i < random_val(1, 100); ++i ) {
			if ( in_bounds( x, y - 1 ) && is_empty( x, y - 1 ) ) {
				particle_t e = particle_ember();
				e.velocity = (gs_vec2){ (f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f };
				write_data( compute_idx( x, y - 1 ), e );
			}
			else if ( in_bounds( x + 1, y - 1 ) && is_empty( x + 1, y - 1 ) ) {
				particle_t e = particle_ember();
				e.velocity = (gs_vec2){ (f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f };
				write_data( compute_idx( x + 1, y - 1 ), e );
			}
			else if ( in_bounds( x - 1, y - 1 ) && is_empty( x - 1, y - 1 ) ) {
				particle_t e = particle_ember();
				e.velocity = (gs_vec2){ (f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 10.f };
				write_data( compute_idx( x - 1, y - 1 ), e );
			}
		}
	}

	// If directly on top of some wall, then replace it
	if ( in_bounds( x, y + 1 ) && ((get_particle_at( x, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x, y + 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x, y + 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x, y + 1 ), particle_fire() );
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						// particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), *p );
						write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x + 1, y + 1 ) && ((get_particle_at( x + 1, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y + 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x + 1, y + 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x + 1, y + 1 ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						// particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), *p );
						write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x - 1, y + 1 ) && ((get_particle_at( x - 1, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y + 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x - 1, y + 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x - 1, y + 1 ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						// particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), *p );
						write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x - 1, y ) && ((get_particle_at( x - 1, y ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x - 1, y ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x - 1, y ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						// particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), *p );
						write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x + 1, y ) && ((get_particle_at( x + 1, y ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x + 1, y ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x + 1, y ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						// particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), *p );
						write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x + 1, y - 1 ) && ((get_particle_at( x + 1, y - 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y - 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x + 1, y - 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x + 1, y - 1 ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						// particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), *p );
						write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x - 1, y - 1 ) && ((get_particle_at( x - 1, y - 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y - 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x - 1, y - 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x - 1, y - 1 ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						// particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), *p );
						write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x, y - 1 ) && is_empty( x, y - 1 ) ) {
		if ( random_val( 0, 50 ) == 0 ) {
			write_data( read_idx, particle_empty() );
			return;
		}
	}

	if ( in_bounds( vi_x, vi_y ) && (is_empty(vi_x, vi_y) || 
									get_particle_at(vi_x, vi_y).id == mat_id_fire ||
									get_particle_at(vi_x, vi_y).id == mat_id_smoke))
	{
		// p->velocity.y -= (gravity * dt );
		particle_t tmp_b = g_world_particle_data[ compute_idx(vi_x, vi_y) ];
		write_data( compute_idx(vi_x, vi_y), *p );
		write_data( read_idx, tmp_b );
	}

	// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y + 1 ) && (( is_empty( x, y + 1 )  || ( g_world_particle_data[ b_idx ].id == mat_id_water ) ) ) ) {
		// p->velocity.y -= (gravity * dt );
		// p->velocity.x = random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		particle_t tmp_b = g_world_particle_data[ b_idx ];
		write_data( b_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y + 1 ) && (( is_empty( x - 1, y + 1 ) || g_world_particle_data[ bl_idx ].id == mat_id_water )) ) {
		// p->velocity.x = random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		// p->velocity.y -= (gravity * dt );
		particle_t tmp_b = g_world_particle_data[ bl_idx ];
		write_data( bl_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + 1, y + 1 ) && (( is_empty( x + 1, y + 1 ) || g_world_particle_data[ br_idx ].id == mat_id_water )) ) {
		// p->velocity.x = random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		// p->velocity.y -= (gravity * dt );
		particle_t tmp_b = g_world_particle_data[ br_idx ];
		write_data( br_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x - 1, y - 1 ) && ( g_world_particle_data[ compute_idx( x - 1, y - 1 ) ].id == mat_id_water  ) ) {
		u32 idx = compute_idx( x - 1, y - 1 );
		// p->velocity.x = random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + 1, y - 1 ) && ( g_world_particle_data[ compute_idx( x + 1, y - 1 ) ].id == mat_id_water  ) ) {
		u32 idx = compute_idx( x + 1, y - 1 );
		// p->velocity.x = random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x, y - 1 ) && ( g_world_particle_data[ compute_idx( x, y - 1 ) ].id == mat_id_water  ) ) {
		u32 idx = compute_idx( x, y - 1 );
		// p->velocity.x = random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		particle_t tmp_b = g_world_particle_data[ idx ];
		write_data( idx, *p );
		write_data( read_idx, tmp_b );
	}
	else {
		// p->velocity.x = random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		write_data( read_idx, *p );
	}
}

void update_lava( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 4;

	if ( p->has_been_updated_this_frame ) {
		return;
	}

	p->has_been_updated_this_frame = true;

	p->velocity.y = gs_clamp( p->velocity.y + ((gravity * dt)), -10.f, 10.f );

	// Change color based on life_time
	if ( random_val( 0, (s32)(p->life_time * 100.f) ) % 200 == 0 ) {
		s32 ran = random_val( 0, 3 );
		switch ( ran ) {
			case 0: p->color = (color_t){ 255, 80, 20, 255 }; break;
			case 1: p->color = (color_t){ 255, 100, 10, 255 }; break;
			case 2: p->color = (color_t){ 255, 50, 0, 255 }; break;
			case 3: p->color = (color_t){ 200, 50, 2, 255 }; break;
		}
	}

	// In water, so create steam and DIE
	// Should also kill the water...
	s32 lx, ly;
	if ( is_in_water( x, y, &lx, &ly ) ) {
		if ( random_val( 0, 1 ) == 0 ) {
			s32 ry = random_val( -5, -1 );
			s32 rx = random_val( -5, 5 );
			for ( s32 i = ry; i > -5; --i ) {
				for ( s32 j = rx; j < 5; ++j ) {
					particle_t p = particle_steam();
					if ( in_bounds( x + j, y + i ) && is_empty( x + j, y + i ) ) {
						particle_t p = particle_steam();
						write_data( compute_idx( x + j, y + i ), p );
					}
				}
			}
			particle_t p = particle_steam();
			write_data( read_idx, particle_empty() );
			write_data( read_idx, p );
			write_data( compute_idx( lx, ly ), particle_stone() );
			return;
		}
	}

	// Otherwise destroy anything that isn't fire or lava...eventually...

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) && ( get_particle_at( x, y + 1 ).id != mat_id_water || get_particle_at( x, y + 1 ).id != mat_id_smoke ) ) {
		p->velocity.y /= 2.f;
	}

	s32 vi_x = x + (s32)p->velocity.x; 
	s32 vi_y = y + (s32)p->velocity.y;

	const s32 spread_rate = 1;
	s32 ran = random_val( 0, 1 );
	s32 r = spread_rate;
	s32 l = -r;
	s32 u = fall_rate;
	s32 v_idx = compute_idx ( x + (s32)p->velocity.x, y + (s32)p->velocity.y );
	s32 b_idx = compute_idx( x, y + u );
	s32 bl_idx = compute_idx( x + l, y + u );
	s32 br_idx = compute_idx( x + r, y + u );
	s32 l_idx = compute_idx( x + l, y );
	s32 r_idx = compute_idx( x + r, y );
	s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;

	const s32 wood_chance = 200;
	const s32 gun_powder_chance = 0;
	const s32 oil_chance = 5;

	// Chance to spawn smoke above
	for ( u32 i = 0; i < random_val( 1, 10 ); ++i ) {
		if ( random_val( 0, 500 ) == 0 ) {
			if ( in_bounds( x, y - 1 ) && is_empty( x, y - 1 ) ) {
				write_data( compute_idx( x, y - 1 ), particle_smoke() );
			}
			else if ( in_bounds( x + 1, y - 1 ) && is_empty( x + 1, y - 1 ) ) {
				write_data( compute_idx( x + 1, y - 1 ), particle_smoke() );
			}
			else if ( in_bounds( x - 1, y - 1 ) && is_empty( x - 1, y - 1 ) ) {
				write_data( compute_idx( x - 1, y - 1 ), particle_smoke() );
			}
		}
	}

	// Spawn embers
	if ( random_val( 0, 250 ) == 0 && p->life_time < 3.f ) {
		for ( u32 i = 0; i < random_val(1, 100); ++i ) {
			if ( in_bounds( x, y - 1 ) && is_empty( x, y - 1 ) ) {
				particle_t e = particle_ember();
				e.velocity = (gs_vec2){ (f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f };
				write_data( compute_idx( x, y - 1 ), e );
			}
			else if ( in_bounds( x + 1, y - 1 ) && is_empty( x + 1, y - 1 ) ) {
				particle_t e = particle_ember();
				e.velocity = (gs_vec2){ (f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f };
				write_data( compute_idx( x + 1, y - 1 ), e );
			}
			else if ( in_bounds( x - 1, y - 1 ) && is_empty( x - 1, y - 1 ) ) {
				particle_t e = particle_ember();
				e.velocity = (gs_vec2){ (f32)random_val(-5, 5) / 5.f, -(f32)random_val(2, 10) / 2.f };
				write_data( compute_idx( x - 1, y - 1 ), e );
			}
		}
	}

	// If directly on top of some wall, then replace it
	if ( in_bounds( x, y + 1 ) && ((get_particle_at( x, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x, y + 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x, y + 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x, y + 1 ), particle_fire() );
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), fp );
						// write_data( read_idx, particle_empty() );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x + 1, y + 1 ) && ((get_particle_at( x + 1, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y + 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x + 1, y + 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x + 1, y + 1 ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), fp );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x - 1, y + 1 ) && ((get_particle_at( x - 1, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y + 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x - 1, y + 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x - 1, y + 1 ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), fp );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x - 1, y ) && ((get_particle_at( x - 1, y ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x - 1, y ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x - 1, y ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), fp );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x + 1, y ) && ((get_particle_at( x + 1, y ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x + 1, y ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x + 1, y ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), fp );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x + 1, y - 1 ) && ((get_particle_at( x + 1, y - 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y - 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x + 1, y - 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x + 1, y - 1 ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), fp );
						break;
					}
				}
			}
		}
	}
	else if ( in_bounds( x - 1, y - 1 ) && ((get_particle_at( x - 1, y - 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y - 1 ).id == mat_id_gunpowder && random_val(0, gun_powder_chance) == 0) ||
									(get_particle_at( x - 1, y - 1 ).id == mat_id_oil && random_val(0, oil_chance) == 0)

		)) {
		write_data( compute_idx( x - 1, y - 1 ), particle_fire() );	
		if ( random_val( 0, 5 ) == 0 ) {
			s32 r = random_val(0, 1);
			for ( s32 i = -3; i < 2; ++i ) {
				for ( s32 j = r ? -3 : 2; r ? j < 2 : j > -3; r ? ++j : --j ) {
					s32 rx = j, ry = i;
					if ( in_bounds( x + rx, y + ry ) && is_empty( x + rx, y + ry ) ) {
						particle_t fp = particle_fire();
						p->life_time += 0.1f;
						write_data( compute_idx( x + rx, y + ry ), fp );
						break;
					}
				}
			}
		}
	}

	// If in water, then need to float upwards
	// s32 lx, ly;
	// if ( is_in_liquid( x, y, &lx, &ly ) && in_bounds( x, y - 1 ) && get_particle_at( x, y - 1 ).id == mat_id_water ) {
	// 	particle_t tmp = get_particle_at( x, y - 1 );
	// 	write_data( compute_idx( x, y - 1 ), *p );
	// 	write_data( read_idx, tmp );
	// 	// return;
	// }

	if ( in_bounds( x + vx, y + vy ) && (is_empty( x + vx, y + vy ) ) ) {
		write_data( v_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_empty( x, y + u ) ) {
		write_data( b_idx, *p );
		write_data( read_idx, particle_empty() );
	} 
	else if ( is_empty( x + r, y + u ) ) {
		write_data( br_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_empty( x + l, y + u ) ) {
		write_data( bl_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else {
		particle_t tmp = *p;
		b32 found = false;

		for ( u32 i = 0; i < fall_rate; ++i ) {
			for ( s32 j = spread_rate; j > 0; --j )
			{
				if ( is_empty( x - j, y + i ) ) {
					write_data( compute_idx( x - j, y + i ), *p );
					write_data( read_idx, particle_empty() );
					found = true;
					break;
				}
				else if ( is_empty( x + j, y + i ) ) {
					write_data( compute_idx( x + j, y + i ), *p );
					write_data( read_idx, particle_empty() );
					found = true;
					break;
				}
			}
		}

		if ( !found ) {
			write_data( read_idx, tmp );
		}
	}
}

void update_oil( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 2;
	u32 spread_rate = 4;

	p->velocity.y = gs_clamp( p->velocity.y + (gravity * dt), -10.f, 10.f );

	p->has_been_updated_this_frame = true;

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	// if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) && get_particle_at( x, y + 1 ).id != mat_id_water ) {
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) ) {
		p->velocity.y /= 2.f;
	}

	// Change color depending on pressure? Pressure would dictate how "deep" the water is, I suppose.
	if ( random_val( 0, (s32)(p->life_time * 100.f) ) % 20 == 0 ) {
		f32 r = (f32)(random_val( 0, 1 )) / 2.f;
		p->color.r = (u8)(gs_interp_linear(0.2f, 0.25f, r) * 255.f);
		p->color.g = (u8)(gs_interp_linear(0.2f, 0.25f, r) * 255.f);
		p->color.b = (u8)(gs_interp_linear(0.2f, 0.25f, r) * 255.f);
	}

	s32 ran = random_val( 0, 1 );
	s32 r = ran ? spread_rate : -spread_rate;
	s32 l = -r;
	s32 u = fall_rate;
	s32 v_idx = compute_idx ( x + (s32)p->velocity.x, y + (s32)p->velocity.y );
	s32 b_idx = compute_idx( x, y + u );
	s32 bl_idx = compute_idx( x + l, y + u );
	s32 br_idx = compute_idx( x + r, y + u );
	s32 l_idx = compute_idx( x + l, y );
	s32 r_idx = compute_idx( x + r, y );
	s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;

	// If in water, then need to float upwards
	// s32 lx, ly;
	// if ( is_in_liquid( x, y, &lx, &ly ) && in_bounds( x, y - 1 ) && get_particle_at( x, y - 1 ).id == mat_id_water ) {
	// 	particle_t tmp = get_particle_at( x, y - 1 );
	// 	write_data( compute_idx( x, y - 1 ), *p );
	// 	write_data( read_idx, tmp );
	// 	// return;
	// }

	if ( in_bounds( x + vx, y + vy ) && (is_empty( x + vx, y + vy ) ) ) {
		write_data( v_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_empty( x, y + u ) ) {
		write_data( b_idx, *p );
		write_data( read_idx, particle_empty() );
	} 
	else if ( is_empty( x + r, y + u ) ) {
		write_data( br_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_empty( x + l, y + u ) ) {
		write_data( bl_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else {
		particle_t tmp = *p;
		b32 found = false;

		for ( u32 i = 0; i < fall_rate; ++i ) {
			for ( s32 j = spread_rate; j > 0; --j )
			{
				if ( is_empty( x - j, y + i ) ) {
					write_data( compute_idx( x - j, y + i ), *p );
					write_data( read_idx, particle_empty() );
					found = true;
					break;
				}
				else if ( is_empty( x + j, y + i ) ) {
					write_data( compute_idx( x + j, y + i ), *p );
					write_data( read_idx, particle_empty() );
					found = true;
					break;
				}
			}
		}

		if ( !found ) {
			write_data( read_idx, tmp );
		}
	}
}

void update_acid( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 2;
	u32 spread_rate = 5;
	s32 lx, ly;

	p->velocity.y = gs_clamp( p->velocity.y + (gravity * dt), -10.f, 10.f );

	p->has_been_updated_this_frame = true;

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	// if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) && get_particle_at( x, y + 1 ).id != mat_id_water ) {
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) ) {
		p->velocity.y /= 2.f;
	}

	// Change color depending on pressure? Pressure would dictate how "deep" the water is, I suppose.
	if ( random_val( 0, (s32)(p->life_time * 100.f) ) % 20 == 0 ) {
		f32 r = (f32)(random_val( 0, 1 )) / 2.f;
		p->color.r = (u8)(gs_interp_linear(0.05f, 0.06f, r) * 255.f);
		p->color.g = (u8)(gs_interp_linear(0.8f, 0.85f, r) * 255.f);
		p->color.b = (u8)(gs_interp_linear(0.1f, 0.12f, r) * 255.f);
	}

	const s32 wood_chance = 100;
	const s32 stone_chance = 300;
	const s32 sand_chance = 50;
	const s32 salt_chance = 20;

	// Random chance to die if in water
	if ( is_in_water( x, y, &lx, &ly ) && random_val( 0, 250 ) == 0 ) {
		write_data( read_idx, particle_empty() );
		return;
	}

	// If directly on top of some wall, then replace it
	if ( in_bounds( x, y + 1 ) && ((get_particle_at( x, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x, y + 1 ).id == mat_id_stone && random_val(0, stone_chance) == 0)
									|| (get_particle_at( x, y + 1 ).id == mat_id_sand && random_val(0, sand_chance) == 0)
									|| (get_particle_at( x, y + 1 ).id == mat_id_salt && random_val(0, salt_chance) == 0)

		)) 
	{
		write_data( compute_idx( x, y + 1 ), *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( in_bounds( x + 1, y + 1 ) && ((get_particle_at( x + 1, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y + 1 ).id == mat_id_stone && random_val(0, stone_chance) == 0) 
									|| (get_particle_at( x + 1, y + 1 ).id == mat_id_sand && random_val(0, sand_chance) == 0)
									|| (get_particle_at( x + 1, y + 1 ).id == mat_id_salt && random_val(0, salt_chance) == 0)

		)) 
	{
		write_data( compute_idx( x + 1, y + 1 ), *p );	
		write_data( read_idx, particle_empty() );
	}
	else if ( in_bounds( x - 1, y + 1 ) && ((get_particle_at( x - 1, y + 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y + 1 ).id == mat_id_stone && random_val(0, stone_chance) == 0)
									|| (get_particle_at( x - 1, y + 1 ).id == mat_id_sand && random_val(0, sand_chance) == 0)
									|| (get_particle_at( x - 1, y + 1 ).id == mat_id_salt && random_val(0, salt_chance) == 0)

		)) 
	{
		write_data( compute_idx( x - 1, y + 1 ), *p );	
		write_data( read_idx, particle_empty() );
	}
	else if ( in_bounds( x - 1, y ) && ((get_particle_at( x - 1, y ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y ).id == mat_id_stone && random_val(0, stone_chance) == 0)
									|| (get_particle_at( x - 1, y ).id == mat_id_sand && random_val(0, sand_chance) == 0)
									|| (get_particle_at( x - 1, y ).id == mat_id_salt && random_val(0, salt_chance) == 0)

		)) 
	{
		write_data( compute_idx( x - 1, y ), *p );	
		write_data( read_idx, particle_empty() );
	}
	else if ( in_bounds( x + 1, y ) && ((get_particle_at( x + 1, y ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y ).id == mat_id_stone && random_val(0, stone_chance) == 0)
									|| (get_particle_at( x + 1, y ).id == mat_id_sand && random_val(0, sand_chance) == 0)
									|| (get_particle_at( x + 1, y ).id == mat_id_salt && random_val(0, sand_chance) == 0)

		)) 
	{
		write_data( compute_idx( x + 1, y ), *p );	
		write_data( read_idx, particle_empty() );
	}
	else if ( in_bounds( x + 1, y - 1 ) && ((get_particle_at( x + 1, y - 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x + 1, y - 1 ).id == mat_id_stone && random_val(0, stone_chance) == 0)
									|| (get_particle_at( x + 1, y - 1 ).id == mat_id_sand && random_val(0, sand_chance) == 0)
									|| (get_particle_at( x + 1, y - 1 ).id == mat_id_salt && random_val(0, salt_chance) == 0)

		)) 
	{
		write_data( compute_idx( x + 1, y - 1 ), *p );	
		write_data( read_idx, particle_empty() );
	}
	else if ( in_bounds( x - 1, y - 1 ) && ((get_particle_at( x - 1, y - 1 ).id == mat_id_wood && random_val( 0, wood_chance ) == 0) ||
									(get_particle_at( x - 1, y - 1 ).id == mat_id_stone && random_val(0, stone_chance) == 0)
									|| (get_particle_at( x - 1, y - 1 ).id == mat_id_sand && random_val(0, sand_chance) == 0)
									|| (get_particle_at( x - 1, y - 1 ).id == mat_id_salt && random_val(0, salt_chance) == 0)

		)) 
	{
		write_data( compute_idx( x - 1, y - 1 ), *p );	
		write_data( read_idx, particle_empty() );
	}

	s32 ran = random_val( 0, 1 );
	s32 r = ran ? spread_rate : -spread_rate;
	s32 l = -r;
	s32 u = fall_rate;
	s32 v_idx = compute_idx ( x + (s32)p->velocity.x, y + (s32)p->velocity.y );
	s32 b_idx = compute_idx( x, y + u );
	s32 bl_idx = compute_idx( x + l, y + u );
	s32 br_idx = compute_idx( x + r, y + u );
	s32 l_idx = compute_idx( x + l, y );
	s32 r_idx = compute_idx( x + r, y );
	s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;

	// If touching wood or stone, destroy it

	if ( in_bounds( x + vx, y + vy ) && (is_empty( x + vx, y + vy ) ) ) {
		write_data( v_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_empty( x, y + u ) ) {
		write_data( b_idx, *p );
		write_data( read_idx, particle_empty() );
	} 
	else if ( is_empty( x + r, y + u ) ) {
		write_data( br_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_empty( x + l, y + u ) ) {
		write_data( bl_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y + u ) && (( is_empty( x, y + u ) ) ) ) {
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x, y + u );
		write_data( b_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + l, y + u ) && (( is_empty( x + l, y + u ) )) ) {
		p->velocity.x = is_in_liquid( x, y, &lx, &ly ) ? 0.f : random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x + l, y + u );
		write_data( bl_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + r, y + u ) && (( is_empty( x + r, y + u ) )) ) {
		p->velocity.x = is_in_liquid( x, y, &lx, &ly ) ? 0.f : random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x + r, y + u );
		write_data( br_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( is_in_liquid( x, y, &lx, &ly ) && random_val( 0, 10 ) == 0 ) {
		particle_t tmp_b = get_particle_at( lx, ly );
		write_data( compute_idx( lx, ly ), *p );
		write_data( read_idx, tmp_b );
	}
	else {
		particle_t tmp = *p;
		b32 found = false;

		// Don't try to spread if something is directly above you?
		if ( completely_surrounded( x, y ) ) {
			write_data( read_idx, tmp );
			return;	
		}
		else {
			for ( u32 i = 0; i < fall_rate; ++i ) {
				for ( s32 j = spread_rate; j > 0; --j )
				{
					if ( in_bounds( x - j, y + i ) && (is_empty( x - j, y + i ) || get_particle_at( x - j, y + i ).id == mat_id_oil ) ) {
						particle_t tmp = get_particle_at( x - j, y + i );
						write_data( compute_idx( x - j, y + i ), *p );
						write_data( read_idx, tmp );
						found = true;
						break;
					}
					if ( in_bounds( x + j, y + i ) && (is_empty( x + j, y + i ) || get_particle_at( x + j, y + i ).id == mat_id_oil ) ) {
						particle_t tmp = get_particle_at( x + j, y + i );
						write_data( compute_idx( x + j, y + i ), *p );
						write_data( read_idx, tmp );
						found = true;
						break;
					}
				}
			}

			if ( !found ) {
				write_data( read_idx, tmp );
			}
		}
	}
}

void update_water( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;
	u32 fall_rate = 2;
	u32 spread_rate = 5;

	p->velocity.y = gs_clamp( p->velocity.y + (gravity * dt), -10.f, 10.f );

	p->has_been_updated_this_frame = true;

	// Just check if you can move directly beneath you. If not, then reset your velocity. God, this is going to blow.
	// if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) && get_particle_at( x, y + 1 ).id != mat_id_water ) {
	if ( in_bounds( x, y + 1 ) && !is_empty( x, y + 1 ) ) {
		p->velocity.y /= 2.f;
	}

	// Change color depending on pressure? Pressure would dictate how "deep" the water is, I suppose.
	if ( random_val( 0, (s32)(p->life_time * 100.f) ) % 20 == 0 ) {
		f32 r = (f32)(random_val( 0, 1 )) / 2.f;
		p->color.r = (u8)(gs_interp_linear(0.1f, 0.15f, r) * 255.f);
		p->color.g = (u8)(gs_interp_linear(0.3f, 0.35f, r) * 255.f);
		p->color.b = (u8)(gs_interp_linear(0.7f, 0.8f, r) * 255.f);
	}

	s32 ran = random_val( 0, 1 );
	s32 r = ran ? spread_rate : -spread_rate;
	s32 l = -r;
	s32 u = fall_rate;
	s32 v_idx = compute_idx ( x + (s32)p->velocity.x, y + (s32)p->velocity.y );
	s32 b_idx = compute_idx( x, y + u );
	s32 bl_idx = compute_idx( x + l, y + u );
	s32 br_idx = compute_idx( x + r, y + u );
	s32 l_idx = compute_idx( x + l, y );
	s32 r_idx = compute_idx( x + r, y );
	s32 vx = (s32)p->velocity.x, vy = (s32)p->velocity.y;
	s32 lx, ly;

	if ( in_bounds( x + vx, y + vy ) && (is_empty( x + vx, y + vy ) ) ) {
		write_data( v_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_empty( x, y + u ) ) {
		write_data( b_idx, *p );
		write_data( read_idx, particle_empty() );
	} 
	else if ( is_empty( x + r, y + u ) ) {
		write_data( br_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	else if ( is_empty( x + l, y + u ) ) {
		write_data( bl_idx, *p );
		write_data( read_idx, particle_empty() );
	}
	// Simple falling, changing the velocity here ruins everything. I need to redo this entire simulation.
	else if ( in_bounds( x, y + u ) && (( is_empty( x, y + u ) || ( g_world_particle_data[ b_idx ].id == mat_id_oil ) ) ) ) {
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x, y + u );
		write_data( b_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + l, y + u ) && (( is_empty( x + l, y + u ) || g_world_particle_data[ bl_idx ].id == mat_id_oil )) ) {
		p->velocity.x = is_in_liquid( x, y, &lx, &ly ) ? 0.f : random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x + l, y + u );
		write_data( bl_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( in_bounds( x + r, y + u ) && (( is_empty( x + r, y + u ) || g_world_particle_data[ br_idx ].id == mat_id_oil )) ) {
		p->velocity.x = is_in_liquid( x, y, &lx, &ly ) ? 0.f : random_val( 0, 1 ) == 0 ? -1.f : 1.f;
		p->velocity.y += (gravity * dt );
		particle_t tmp_b = get_particle_at( x + r, y + u );
		write_data( br_idx, *p );
		write_data( read_idx, tmp_b );
	}
	else if ( is_in_liquid( x, y, &lx, &ly ) && random_val( 0, 10 ) == 0 ) {
		particle_t tmp_b = get_particle_at( lx, ly );
		write_data( compute_idx( lx, ly ), *p );
		write_data( read_idx, tmp_b );
	}
	else {
		particle_t tmp = *p;
		b32 found = false;

		// Don't try to spread if something is directly above you?
		if ( completely_surrounded( x, y ) ) {
			write_data( read_idx, tmp );
			return;	
		}
		else {
			for ( u32 i = 0; i < fall_rate; ++i ) {
				for ( s32 j = spread_rate; j > 0; --j )
				{
					if ( in_bounds( x - j, y + i ) && (is_empty( x - j, y + i ) || get_particle_at( x - j, y + i ).id == mat_id_oil ) ) {
						particle_t tmp = get_particle_at( x - j, y + i );
						write_data( compute_idx( x - j, y + i ), *p );
						write_data( read_idx, tmp );
						found = true;
						break;
					}
					if ( in_bounds( x + j, y + i ) && (is_empty( x + j, y + i ) || get_particle_at( x + j, y + i ).id == mat_id_oil ) ) {
						particle_t tmp = get_particle_at( x + j, y + i );
						write_data( compute_idx( x + j, y + i ), *p );
						write_data( read_idx, tmp );
						found = true;
						break;
					}
				}
			}

			if ( !found ) {
				write_data( read_idx, tmp );
			}
		}
	}
}

void update_default( u32 w, u32 h )
{
	u32 read_idx = compute_idx( w, h );
	write_data( read_idx, get_particle_at( w, h ) );
}

particle_t particle_empty()
{
	particle_t p = {0};
	p.id = mat_id_empty;
	p.color = mat_col_empty;
	return p;
}

particle_t particle_sand()
{
	particle_t p = {0};
	p.id = mat_id_sand;
	// Random sand color
	f32 r = (f32)(random_val( 0, 10 )) / 10.f;
	p.color.r = (u8)(gs_interp_linear(0.8f, 1.f, r) * 255.f);
	p.color.g = (u8)(gs_interp_linear(0.5f, 0.6f, r) * 255.f);
	p.color.b = (u8)(gs_interp_linear(0.2f, 0.25f, r) * 255.f);
	p.color.a = 255;
	return p;
}

particle_t particle_water()
{
	particle_t p = {0};
	p.id = mat_id_water;
	f32 r = (f32)(random_val( 0, 1 )) / 2.f;
	p.color.r = (u8)(gs_interp_linear(0.1f, 0.15f, r) * 255.f);
	p.color.g = (u8)(gs_interp_linear(0.3f, 0.35f, r) * 255.f);
	p.color.b = (u8)(gs_interp_linear(0.7f, 0.8f, r) * 255.f);
	p.color.a = 255;
	return p;
}

particle_t particle_salt()
{
	particle_t p = {0};
	p.id = mat_id_salt;
	f32 r = (f32)(random_val( 0, 1 )) / 2.f;
	p.color.r = (u8)(gs_interp_linear(0.9f, 1.0f, r) * 255.f);
	p.color.g = (u8)(gs_interp_linear(0.8f, 0.85f, r) * 255.f);
	p.color.b = (u8)(gs_interp_linear(0.8f, 0.9f, r) * 255.f);
	p.color.a = 255;
	return p;
}

particle_t particle_wood()
{
	particle_t p = {0};
	p.id = mat_id_wood;
	f32 r = (f32)(random_val( 0, 1 )) / 2.f;
	p.color.r = (u8)(gs_interp_linear(0.23f, 0.25f, r) * 255.f);
	p.color.g = (u8)(gs_interp_linear(0.15f, 0.18f, r) * 255.f);
	p.color.b = (u8)(gs_interp_linear(0.02f, 0.03f, r) * 255.f);
	p.color.a = 255;
	return p;
}

particle_t particle_gunpowder()
{
	particle_t p = {0};
	p.id = mat_id_gunpowder;
	f32 r = (f32)(random_val( 0, 1 )) / 2.f;
	p.color.r = (u8)(gs_interp_linear(0.15f, 0.2f, r) * 255.f);
	p.color.g = (u8)(gs_interp_linear(0.15f, 0.2f, r) * 255.f);
	p.color.b = (u8)(gs_interp_linear(0.15f, 0.2f, r) * 255.f);
	p.color.a = 255;
	return p;
}

particle_t particle_oil()
{
	particle_t p = {0};
	p.id = mat_id_oil;
	f32 r = (f32)(random_val( 0, 1 )) / 2.f;
	p.color.r = (u8)(gs_interp_linear(0.12f, 0.15f, r) * 255.f);
	p.color.g = (u8)(gs_interp_linear(0.10f, 0.12f, r) * 255.f);
	p.color.b = (u8)(gs_interp_linear(0.08f, 0.10f, r) * 255.f);
	p.color.a = 255;
	return p;
}

particle_t particle_fire()
{
	particle_t p = {0};
	p.id = mat_id_fire;
	p.color = mat_col_fire;
	return p;
}

particle_t particle_lava()
{
	particle_t p = {0};
	p.id = mat_id_lava;
	p.color = mat_col_fire;
	return p;
}

particle_t particle_ember()
{
	particle_t p = {0};
	p.id = mat_id_ember;
	p.color = mat_col_ember;
	return p;
}

particle_t particle_smoke()
{
	particle_t p = {0};
	p.id = mat_id_smoke;
	p.color = mat_col_smoke;
	return p;
}

particle_t particle_steam()
{
	particle_t p = {0};
	p.id = mat_id_steam;
	p.color = mat_col_steam;
	return p;
}

particle_t particle_stone()
{
	particle_t p = {0};
	p.id = mat_id_stone;
	f32 r = (f32)(random_val( 0, 1 )) / 2.f;
	p.color.r = (u8)(gs_interp_linear(0.5f, 0.65f, r) * 255.f);
	p.color.g = (u8)(gs_interp_linear(0.5f, 0.65f, r) * 255.f);
	p.color.b = (u8)(gs_interp_linear(0.5f, 0.65f, r) * 255.f);
	p.color.a = 255;
	return p;
}

particle_t particle_acid()
{
	particle_t p = {0};
	p.id = mat_id_acid;
	f32 r = (f32)(random_val( 0, 1 )) / 2.f;
	p.color.r = (u8)(gs_interp_linear(0.05f, 0.06f, r) * 255.f);
	p.color.g = (u8)(gs_interp_linear(0.8f, 0.85f, r) * 255.f);
	p.color.b = (u8)(gs_interp_linear(0.1f, 0.12f, r) * 255.f);
	p.color.a = 200;
	return p;
}


