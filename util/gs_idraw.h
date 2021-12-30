
/*================================================================
	* Copyright: 2020 John Jackson
	* GSIDraw: Immediate Mode Drawing Util for Gunslinger 
	* File: gs_idraw.h
	All Rights Reserved
=================================================================*/

#ifndef GS_IDRAW_H
#define GS_IDRAW_H

/*
	USAGE: (IMPORTANT)

	=================================================================================================================

	Before including, define the gunslinger immediate draw implementation like this:

	    #define GS_IMMEDIATE_DRAW_IMPL

    in EXACTLY ONE C or C++ file that includes this header, BEFORE the
    include, like this:

		#define GS_IMMEDIATE_DRAW_IMPL
		#include "gs_idraw.h"

    All other files should just #include "gs_idraw.h" without the #define.

    MUST include "gs.h" and declare GS_IMPL BEFORE this file, since this file relies on that:

    	#define GS_IMPL
    	#include "gs.h"

    	#define GS_IMMEDIATE_DRAW_IMPL
    	#include "gs_idraw.h"

    TODO (john): 
		* Convert flush command to push back commands
		* On final flush, request update for vertex/index buffer data
		* Then iterate commands to submit pipelines + state to gfx backend

	================================================================================================================
*/

/*==== Interface ====*/

gs_enum_decl(gsi_matrix_type,
	GSI_MATRIX_MODELVIEW,
	GSI_MATRIX_PROJECTION
);

// Need a configurable pipeline matrix
/*
	depth | stencil | face cull | blend | prim
	e/d 	e/d 		e/d 		e/d 	l/t

	2 ^ 5 = 32 generated pipeline choices.
*/

// Hash bytes of state attr struct to get index key for pipeline
typedef struct gsi_pipeline_state_attr_t
{
	uint16_t depth_enabled;	
	uint16_t stencil_enabled;	
	uint16_t blend_enabled;	
	uint16_t face_cull_enabled;
	uint16_t prim_type;
} gsi_pipeline_state_attr_t;

typedef struct gs_immediate_vert_t
{
	gs_vec3 position;	
	gs_vec2 uv;
	gs_color_t color;
} gs_immediate_vert_t;

typedef struct gs_immediate_cache_t
{
	/* Pipeline stack */
	gs_dyn_array(gs_handle(gs_graphics_pipeline_t)) pipelines;
	/* Modelview stack*/
	gs_dyn_array(gs_mat4) modelview;
	/* Projection stack*/
	gs_dyn_array(gs_mat4) projection;
	/* Mode stack*/
	gs_dyn_array(gsi_matrix_type) modes;
	/* UV */
	gs_vec2 uv;
	/* Color */
	gs_color_t color;
	/* Texture */
	gs_handle(gs_graphics_texture_t) texture;
	// Cached pipeline state attr
	gsi_pipeline_state_attr_t pipeline;

} gs_immediate_cache_t;

typedef struct gs_immediate_draw_static_data_t
{
	/* Default texture */
	gs_handle(gs_graphics_texture_t) tex_default;
	/* Default font */
	gs_asset_font_t font_default;
	/* Pipeline state matrix table */
	gs_hash_table(gsi_pipeline_state_attr_t, gs_handle(gs_graphics_pipeline_t)) pipeline_table;
	/* Uniform buffer */
	gs_handle(gs_graphics_uniform_t) uniform;
	/* Uniform sampler */
	gs_handle(gs_graphics_uniform_t) sampler;

} gs_immediate_draw_static_data_t;

typedef struct gs_immediate_draw_t
{
	/* Handle to vertex buffer resource */
	gs_handle(gs_graphics_vertex_buffer_t) vbo;
	/* Handle to index buffer resource */
	gs_handle(gs_graphics_index_buffer_t) ibo;
	/* Dynamic array of vertex data to update */
	gs_dyn_array(gs_immediate_vert_t) vertices;
	/* Dynamic array of index data to update */
	gs_dyn_array(uint16_t) indices;
	/* Cache */
	gs_immediate_cache_t cache;
	/* Internal Command Buffer */
	gs_command_buffer_t commands;
    /* Window handle this context is bound to */
    uint32_t window_handle;

} gs_immediate_draw_t;

#ifndef GS_NO_SHORT_NAME
	typedef gs_immediate_draw_t gsid;
	#define gsi_create gs_immediate_draw_new
	#define gsi_free   gs_immediate_draw_free
#endif

// Create / Init / Shutdown / Free
GS_API_DECL gs_immediate_draw_t gs_immediate_draw_new(uint32_t window_handle);
GS_API_DECL void                gs_immediate_draw_free(gs_immediate_draw_t* gsi);

// Get pipeline from state
GS_API_DECL gs_handle(gs_graphics_pipeline_t) gsi_get_pipeline(gs_immediate_draw_t* gsi, gsi_pipeline_state_attr_t state);

// Get default font asset pointer
GS_API_DECL gs_asset_font_t* gsi_default_font();

// Core Vertex Functions
GS_API_DECL void gsi_begin(gs_immediate_draw_t* gsi, gs_graphics_primitive_type type);
GS_API_DECL void gsi_end(gs_immediate_draw_t* gsi);
GS_API_DECL void gsi_tc2f(gs_immediate_draw_t* gsi, float u, float v);
GS_API_DECL void gsi_tc2fv(gs_immediate_draw_t* gsi, gs_vec2 uv);
GS_API_DECL void gsi_c4ub(gs_immediate_draw_t* gsi, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
GS_API_DECL void gsi_v2f(gs_immediate_draw_t* gsi, float x, float y);
GS_API_DECL void gsi_v2fv(gs_immediate_draw_t* gsi, gs_vec2 v);
GS_API_DECL void gsi_v3f(gs_immediate_draw_t* gsi, float x, float y, float z);
GS_API_DECL void gsi_v3fv(gs_immediate_draw_t* gsi, gs_vec3 v);
GS_API_DECL void gsi_flush(gs_immediate_draw_t* gsi);
GS_API_DECL void gsi_texture(gs_immediate_draw_t* gsi, gs_handle(gs_graphics_texture_t) texture);

// Core pipeline functions
GS_API_DECL void gsi_blend_enabled(gs_immediate_draw_t* gsi, bool enabled);
GS_API_DECL void gsi_depth_enabled(gs_immediate_draw_t* gsi, bool enabled);
GS_API_DECL void gsi_stencil_enabled(gs_immediate_draw_t* gsi, bool enabled);
GS_API_DECL void gsi_face_cull_enabled(gs_immediate_draw_t* gsi, bool enabled);
GS_API_DECL void gsi_defaults(gs_immediate_draw_t* gsi);

// View/Scissor commands
GS_API_DECL void gsi_set_view_scissor(gs_immediate_draw_t* gsi, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

// Final Submit / Merge
GS_API_DECL void gsi_draw(gs_immediate_draw_t* gsi, gs_command_buffer_t* cb);
GS_API_DECL void gsi_render_pass_submit(gs_immediate_draw_t* gsi, gs_command_buffer_t* cb, gs_color_t clear_color);
GS_API_DECL void gsi_render_pass_submit_ex(gs_immediate_draw_t* gsi, gs_command_buffer_t* cb, gs_graphics_clear_action_t* action);

// Core Matrix Functions
GS_API_DECL void gsi_push_matrix(gs_immediate_draw_t* gsi, gsi_matrix_type type);
GS_API_DECL void gsi_pop_matrix(gs_immediate_draw_t* gsi);
GS_API_DECL void gsi_matrix_mode(gs_immediate_draw_t* gsi, gsi_matrix_type type);
GS_API_DECL void gsi_load_matrix(gs_immediate_draw_t* gsi, gs_mat4 m);
GS_API_DECL void gsi_mul_matrix(gs_immediate_draw_t* gsi, gs_mat4 m);
GS_API_DECL void gsi_perspective(gs_immediate_draw_t* gsi, float fov, float aspect, float near, float far);
GS_API_DECL void gsi_ortho(gs_immediate_draw_t* gsi, float left, float right, float bottom, float top, float near, float far);
GS_API_DECL void gsi_rotatef(gs_immediate_draw_t* gsi, float angle, float x, float y, float z);
GS_API_DECL void gsi_rotatefv(gs_immediate_draw_t* gsi, float angle, gs_vec3 v);
GS_API_DECL void gsi_transf(gs_immediate_draw_t* gsi, float x, float y, float z);
GS_API_DECL void gsi_scalef(gs_immediate_draw_t* gsi, float x, float y, float z);

// Camera Utils
GS_API_DECL void gsi_camera(gs_immediate_draw_t* gsi, gs_camera_t* cam);
GS_API_DECL void gsi_camera2D(gs_immediate_draw_t* gsi);
GS_API_DECL void gsi_camera3D(gs_immediate_draw_t* gsi);

// Primitive Drawing Util
GS_API_DECL void gsi_triangle(gs_immediate_draw_t* gsi, float x0, float y0, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_trianglev(gs_immediate_draw_t* gsi, gs_vec2 a, gs_vec2 b, gs_vec2 c, gs_color_t color, gs_graphics_primitive_type type);
GS_API_DECL void gsi_trianglex(gs_immediate_draw_t* gsi, float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float u0, float v0, float u1, float v1, float u2, float v2, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_trianglevx(gs_immediate_draw_t* gsi, gs_vec3 v0, gs_vec3 v1, gs_vec3 v2, gs_vec2 uv0, gs_vec2 uv1, gs_vec2 uv2, gs_color_t color, gs_graphics_primitive_type type);
GS_API_DECL void gsi_trianglevxmc(gs_immediate_draw_t* gsi, gs_vec3 v0, gs_vec3 v1, gs_vec3 v2, gs_vec2 uv0, gs_vec2 uv1, gs_vec2 uv2, gs_color_t c0, gs_color_t c1, gs_color_t c2, gs_graphics_primitive_type type);
GS_API_DECL void gsi_line(gs_immediate_draw_t* gsi, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
GS_API_DECL void gsi_linev(gs_immediate_draw_t* gsi, gs_vec2 v0, gs_vec2 v1, gs_color_t c);
GS_API_DECL void gsi_line3D(gs_immediate_draw_t* gsi, float x0, float y0, float z0, float x1, float y1, float z1, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
GS_API_DECL void gsi_line3Dv(gs_immediate_draw_t* gsi, gs_vec3 s, gs_vec3 e, gs_color_t color);
GS_API_DECL void gsi_line3Dmc(gs_immediate_draw_t* gsi, float x0, float y0, float z0, float x1, float y1, float z1, uint8_t r0, uint8_t g0, uint8_t b0, uint8_t a0, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t a1);

// Shape Drawing Util
GS_API_DECL void gsi_rect(gs_immediate_draw_t* gsi, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_rectv(gs_immediate_draw_t* gsi, gs_vec2 bl, gs_vec2 tr, gs_color_t color, gs_graphics_primitive_type type);
GS_API_DECL void gsi_rectx(gs_immediate_draw_t* gsi, float l, float b, float r, float t, float u0, float v0, float u1, float v1, uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_rectvx(gs_immediate_draw_t* gsi, gs_vec2 bl, gs_vec2 tr, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color, gs_graphics_primitive_type type);
GS_API_DECL void gsi_rectvd(gs_immediate_draw_t* gsi, gs_vec2 xy, gs_vec2 wh, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color, gs_graphics_primitive_type type);
GS_API_DECL void gsi_rect3Dv(gs_immediate_draw_t* gsi, gs_vec3 min, gs_vec3 max, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color, gs_graphics_primitive_type type);
GS_API_DECL void gsi_circle(gs_immediate_draw_t* gsi, float cx, float cy, float radius, int32_t segments, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_circlevx(gs_immediate_draw_t* gsi, gs_vec3 c, float radius, int32_t segments, gs_color_t color, gs_graphics_primitive_type type);
GS_API_DECL void gsi_circle_sector(gs_immediate_draw_t* gsi, float cx, float cy, float radius, int32_t start_angle, int32_t end_angle, int32_t segments, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_circle_sectorvx(gs_immediate_draw_t* gsi, gs_vec3 c, float radius, int32_t start_angle, int32_t end_angle, int32_t segments, gs_color_t color, gs_graphics_primitive_type type);
GS_API_DECL void gsi_box(gs_immediate_draw_t* gsi, float x0, float y0, float z0, float hx, float hy, float hz, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_sphere(gs_immediate_draw_t* gsi, float cx, float cy, float cz, float radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_bezier(gs_immediate_draw_t* gsi, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
GS_API_DECL void gsi_cylinder(gs_immediate_draw_t* gsi, float x, float y, float z, float r_top, float r_bottom, float height, int32_t sides, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);
GS_API_DECL void gsi_cone(gs_immediate_draw_t* gsi, float x, float y, float z, float radius, float height, int32_t sides, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type);

// Draw planes/poly groups

// Text Drawing Util
GS_API_DECL void gsi_text(gs_immediate_draw_t* gsi, float x, float y, const char* text, const gs_asset_font_t* fp, bool32_t flip_vertical, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// Private Internal Utilities (Not user facing)
GS_API_DECL const char* GSGetDefaultCompressedFontDataTTFBase85();
GS_API_DECL void GSDecode85(const unsigned char* src, unsigned char* dst);
GS_API_DECL unsigned int GSDecode85Byte(char c);
GS_API_DECL unsigned int gs_decompress_length(const unsigned char* input);
GS_API_DECL unsigned int gs_decompress(unsigned char* output, unsigned char* input, unsigned int length);

/*==== Implementation ====*/

#ifdef GS_IMMEDIATE_DRAW_IMPL

// Global instance of immediate draw static data
gs_immediate_draw_static_data_t* g_gsi = NULL;

#define GSI() g_gsi

#ifndef gsi_smooth_circle_error_rate
    #define gsi_smooth_circle_error_rate  0.5f
#endif

const f32 gsi_deg2rad = (f32)GS_PI / 180.f;

// Shaders
#if (defined GS_PLATFORM_WEB || defined GS_PLATFORM_ANDROID)
    #define GSI_GL_VERSION_STR "#version 300 es\n"
#else
    #define GSI_GL_VERSION_STR "#version 330 core\n"
#endif 

const char* gsi_v_fillsrc =
GSI_GL_VERSION_STR
"precision mediump float;\n"
"layout(location = 0) in vec3 a_position;\n"
"layout(location = 1) in vec2 a_uv;\n"
"layout(location = 2) in vec4 a_color;\n"
"uniform mat4 u_mvp;\n"
"out vec2 uv;\n"
"out vec4 color;\n"
"void main() {\n"
"  gl_Position = u_mvp * vec4(a_position, 1.0);\n"
"  uv = a_uv;\n"
"  color = a_color;\n"
"}\n";

const char* gsi_f_fillsrc =
GSI_GL_VERSION_STR
"precision mediump float;\n"
"in vec2 uv;\n"
"in vec4 color;\n"
"uniform sampler2D u_tex;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"  frag_color = color * texture(u_tex, uv);\n"
"}\n";

gsi_pipeline_state_attr_t gsi_pipeline_state_default()
{
	gsi_pipeline_state_attr_t attr = gs_default_val();
	attr.depth_enabled = false;
	attr.stencil_enabled = false;
	attr.blend_enabled = true;
	attr.face_cull_enabled = false;
	attr.prim_type = (uint16_t)GS_GRAPHICS_PRIMITIVE_TRIANGLES;
	return attr;
}

void gsi_reset(gs_immediate_draw_t* gsi)
{
	gs_command_buffer_clear(&gsi->commands);
	gs_dyn_array_clear(gsi->vertices);	
	gs_dyn_array_clear(gsi->indices);	
	gs_dyn_array_clear(gsi->cache.modelview);
	gs_dyn_array_clear(gsi->cache.projection);
	gs_dyn_array_clear(gsi->cache.pipelines);
	gs_dyn_array_clear(gsi->cache.modes);

	gs_dyn_array_push(gsi->cache.modelview, gs_mat4_identity());
	gs_dyn_array_push(gsi->cache.projection, gs_mat4_identity());
	gs_dyn_array_push(gsi->cache.modes, GSI_MATRIX_MODELVIEW);

	gsi->cache.texture = GSI()->tex_default;
	gsi->cache.pipeline = gsi_pipeline_state_default();
	gsi->cache.pipeline.prim_type = 0x00;
	gsi->cache.uv = gs_v2(0.f, 0.f);
	gsi->cache.color = GS_COLOR_WHITE;
}

void gs_immediate_draw_static_data_init()
{
    GSI() = gs_malloc_init(gs_immediate_draw_static_data_t);

	// Create uniform buffer
	gs_graphics_uniform_layout_desc_t uldesc = gs_default_val();
	uldesc.type = GS_GRAPHICS_UNIFORM_MAT4;
	gs_graphics_uniform_desc_t udesc = gs_default_val();
	memcpy(udesc.name, "u_mvp", 64);
	udesc.layout = &uldesc;
	GSI()->uniform = gs_graphics_uniform_create(&udesc);

	// Create sampler buffer 
	gs_graphics_uniform_layout_desc_t sldesc = gs_default_val(); 
	sldesc.type = GS_GRAPHICS_UNIFORM_SAMPLER2D;
	gs_graphics_uniform_desc_t sbdesc = gs_default_val();
	memcpy(sbdesc.name, "u_tex", 64);
	sbdesc.layout = &sldesc;
	GSI()->sampler = gs_graphics_uniform_create(&sbdesc); 

	// Create default texture (4x4 white) 
	gs_color_t pixels[16] = gs_default_val();
	memset(pixels, 255, 16 * sizeof(gs_color_t));
	
	gs_graphics_texture_desc_t tdesc = gs_default_val();
	tdesc.width = 4;
	tdesc.height = 4;
	tdesc.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8;
	tdesc.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;
	tdesc.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;	
	tdesc.data = pixels;

	GSI()->tex_default = gs_graphics_texture_create(&tdesc);

	// Create shader
	gs_graphics_shader_source_desc_t vsrc; vsrc.type = GS_GRAPHICS_SHADER_STAGE_VERTEX; vsrc.source = gsi_v_fillsrc;
	gs_graphics_shader_source_desc_t fsrc; fsrc.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT; fsrc.source = gsi_f_fillsrc;
	gs_graphics_shader_source_desc_t gsi_sources[] = {
		vsrc, fsrc
	};

	gs_graphics_shader_desc_t sdesc = gs_default_val();
	sdesc.sources = gsi_sources;
	sdesc.size = sizeof(gsi_sources);
	memcpy(sdesc.name, "gs_immediate_default_fill_shader", 64);

	// Vertex attr layout
    gs_graphics_vertex_attribute_desc_t gsi_vattrs[3] = gs_default_val();
    gsi_vattrs[0].format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3; memcpy(gsi_vattrs[0].name, "a_position", 64);
    gsi_vattrs[1].format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2; memcpy(gsi_vattrs[1].name, "a_uv", 64);
    gsi_vattrs[2].format = GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4; memcpy(gsi_vattrs[2].name, "a_color", 64);

	// Iterate through attribute list, then create custom pipelines requested.
	gs_handle(gs_graphics_shader_t) shader = gs_graphics_shader_create(&sdesc);

	// Pipelines
	for (uint16_t d = 0; d < 2; ++d) // Depth
		for (uint16_t s = 0; s < 2; ++s) // Stencil
			for (uint16_t b = 0; b < 2; ++b) // Blend
				for (uint16_t f = 0; f < 2; ++f) // Face Cull
					for (uint16_t p = 0; p < 2; ++p) // Prim Type
	{
		gsi_pipeline_state_attr_t attr = gs_default_val();

		attr.depth_enabled 		= d;
		attr.stencil_enabled 	= s;
		attr.blend_enabled 		= b;
		attr.face_cull_enabled  = f;
		attr.prim_type 			= p ? (uint16_t)GS_GRAPHICS_PRIMITIVE_TRIANGLES : (uint16_t)GS_GRAPHICS_PRIMITIVE_LINES;

		// Create new pipeline based on this arrangement
		gs_graphics_pipeline_desc_t pdesc = gs_default_val();
		pdesc.raster.shader = shader;
		pdesc.raster.index_buffer_element_size = sizeof(uint16_t);
		pdesc.raster.face_culling = attr.face_cull_enabled ? GS_GRAPHICS_FACE_CULLING_BACK : (gs_graphics_face_culling_type)0x00;
		pdesc.raster.primitive = (gs_graphics_primitive_type)attr.prim_type; 
		pdesc.blend.func = attr.blend_enabled ? GS_GRAPHICS_BLEND_EQUATION_ADD : (gs_graphics_blend_equation_type)0x00;
		pdesc.blend.src = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA;
		pdesc.blend.dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA;
		pdesc.depth.func = d ? GS_GRAPHICS_DEPTH_FUNC_LESS : (gs_graphics_depth_func_type)0x00;
		pdesc.layout.attrs = gsi_vattrs;
		pdesc.layout.size = sizeof(gsi_vattrs);

		gs_handle(gs_graphics_pipeline_t) hndl = gs_graphics_pipeline_create(&pdesc);
		gs_hash_table_insert(GSI()->pipeline_table, attr, hndl);
	} 

	// Create default font
	gs_asset_font_t* f = &GSI()->font_default;
	stbtt_fontinfo font = gs_default_val();
	const char* compressed_ttf_data_base85 = GSGetDefaultCompressedFontDataTTFBase85();
	s32 compressed_ttf_size = (((s32)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf_data = gs_malloc((usize)compressed_ttf_size);
    GSDecode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf_data);
    const u32 buf_decompressed_size = gs_decompress_length((unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char*)gs_malloc(buf_decompressed_size);
    gs_decompress(buf_decompressed_data, (unsigned char*)compressed_ttf_data, (u32)compressed_ttf_size);

	const u32 w = 512;
	const u32 h = 512;
	const u32 num_comps = 4;
	u8* alpha_bitmap = (u8*)gs_malloc(w * h);
	u8* flipmap = (u8*)gs_malloc(w * h * num_comps);
	memset(alpha_bitmap, 0, w * h);
	memset(flipmap, 0, w * h * num_comps);
   	s32 v = stbtt_BakeFontBitmap((u8*)buf_decompressed_data, 0, 13.f, alpha_bitmap, w, h, 32, 96, (stbtt_bakedchar*)f->glyphs); // no guarantee this fits!

   	// Flip texture
   	u32 r = h - 1;
   	for (u32 i = 0; i < h; ++i)
   	{
   		for (u32 j = 0; j < w; ++j)
   		{
   			u32 i0 = i * w + j;
   			u32 i1 = i * w * num_comps + j * num_comps;
   			u8 a = alpha_bitmap[i0];
   			flipmap[i1 + 0] = 255;
   			flipmap[i1 + 1] = 255;
   			flipmap[i1 + 2] = 255;
   			flipmap[i1 + 3] = a;
   		}
   		r--;
   	}

   	gs_graphics_texture_desc_t desc = gs_default_val();
   	desc.width = w;
   	desc.height = h;
   	desc.data = flipmap;
   	desc.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8;
   	desc.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;
   	desc.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;

   	// Generate atlas texture for bitmap with bitmap data
   	f->texture.hndl = gs_graphics_texture_create(&desc);
   	f->texture.desc = desc;
   	f->texture.desc.data = NULL;

    gs_free(compressed_ttf_data);
   	gs_free(buf_decompressed_data);
   	gs_free(alpha_bitmap);
   	gs_free(flipmap);
}

// Create / Init / Shutdown / Free
gs_immediate_draw_t gs_immediate_draw_new(uint32_t window_handle)
{
    if (!GSI())
    {  
        // Construct GSI
        gs_immediate_draw_static_data_init();
    }

	gs_immediate_draw_t gsi = gs_default_val();
	memset(&gsi, 0, sizeof(gsi));

	// Init cache
	gsi.cache.color = GS_COLOR_WHITE;

	// Init command buffer
	gsi.commands = gs_command_buffer_new();	// Not totally sure on the syntax for new vs. create

    // Set window handle
    gsi.window_handle = window_handle; 

	// Create vertex buffer 
	gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
	vdesc.data = NULL;
	vdesc.size = 0;
	vdesc.usage = GS_GRAPHICS_BUFFER_USAGE_STREAM;

	gsi.vbo = gs_graphics_vertex_buffer_create(&vdesc); 

	// Set up cache 
	gsi_reset(&gsi);

	return gsi;
}

void gs_immediate_draw_free(gs_immediate_draw_t* gsi)
{
	// Free all data
}

GS_API_DECL gs_asset_font_t* gsi_default_font()
{
    if (GSI()) return &GSI()->font_default;
    return NULL;
}

gs_handle(gs_graphics_pipeline_t) gsi_get_pipeline(gs_immediate_draw_t* gsi, gsi_pipeline_state_attr_t state)
{
	// Bind pipeline
	gs_assert(gs_hash_table_key_exists(GSI()->pipeline_table, state));
	return gs_hash_table_get(GSI()->pipeline_table, state);
}

void gs_immediate_draw_set_pipeline(gs_immediate_draw_t* gsi)
{
	// Check validity 
	if (gsi->cache.pipeline.prim_type != (uint16_t)GS_GRAPHICS_PRIMITIVE_TRIANGLES && gsi->cache.pipeline.prim_type != (uint16_t)GS_GRAPHICS_PRIMITIVE_LINES) 
	{
		gsi->cache.pipeline.prim_type = (uint16_t)GS_GRAPHICS_PRIMITIVE_TRIANGLES;
	}
	gsi->cache.pipeline.depth_enabled = gs_clamp(gsi->cache.pipeline.depth_enabled, 0, 1);
	gsi->cache.pipeline.stencil_enabled = gs_clamp(gsi->cache.pipeline.stencil_enabled, 0, 1);
	gsi->cache.pipeline.face_cull_enabled = gs_clamp(gsi->cache.pipeline.face_cull_enabled, 0, 1);
	gsi->cache.pipeline.blend_enabled = gs_clamp(gsi->cache.pipeline.blend_enabled, 0, 1);

	// Bind pipeline
	gs_assert(gs_hash_table_key_exists(GSI()->pipeline_table, gsi->cache.pipeline));
	gs_graphics_bind_pipeline(&gsi->commands, gs_hash_table_get(GSI()->pipeline_table, gsi->cache.pipeline));
}

/* Core Vertex Functions */
void gsi_begin(gs_immediate_draw_t* gsi, gs_graphics_primitive_type type)
{
	switch (type) {
		default:
		case GS_GRAPHICS_PRIMITIVE_TRIANGLES: type = GS_GRAPHICS_PRIMITIVE_TRIANGLES; break;
		case GS_GRAPHICS_PRIMITIVE_LINES:     type = GS_GRAPHICS_PRIMITIVE_LINES;     break;
	}

	// Push a new pipeline?
	if (gsi->cache.pipeline.prim_type == type) {
		return;
	}

	// Otherwise, we need to flush previous content
	gsi_flush(gsi);

	// Set primitive type
	gsi->cache.pipeline.prim_type = type;	

	// Bind pipeline
	gs_immediate_draw_set_pipeline(gsi);
}

void gsi_end(gs_immediate_draw_t* gsi)
{
	// Not sure what to do here...
}

void gsi_flush(gs_immediate_draw_t* gsi)
{
	// Don't flush if verts empty
	if (gs_dyn_array_empty(gsi->vertices)) {
		return;
	}

	// Set up mvp matrix
	gs_mat4 mv = gsi->cache.modelview[gs_dyn_array_size(gsi->cache.modelview) - 1];
	gs_mat4 proj = gsi->cache.projection[gs_dyn_array_size(gsi->cache.projection) - 1];
	gs_mat4 mvp = gs_mat4_mul(proj, mv);

	// Update vertex buffer (command buffer version)
	gs_graphics_vertex_buffer_desc_t vdesc = gs_default_val();
	vdesc.data = gsi->vertices;
	vdesc.size = gs_dyn_array_size(gsi->vertices) * sizeof(gs_immediate_vert_t);
	vdesc.usage = GS_GRAPHICS_BUFFER_USAGE_STREAM;

	gs_graphics_vertex_buffer_request_update(&gsi->commands, gsi->vbo, &vdesc);

	// Set up all binding data
	gs_graphics_bind_vertex_buffer_desc_t vbuffer = gs_default_val();
	vbuffer.buffer = gsi->vbo;

	gs_graphics_bind_uniform_desc_t ubinds[2] = gs_default_val();
	ubinds[0].uniform = GSI()->uniform; ubinds[0].data = &mvp;
	ubinds[1].uniform = GSI()->sampler; ubinds[1].data = &gsi->cache.texture; ubinds[1].binding = 0;

    // Bindings for all buffers: vertex, uniform, sampler
    gs_graphics_bind_desc_t binds = gs_default_val();
   	binds.vertex_buffers.desc = &vbuffer; 
   	binds.uniforms.desc = ubinds;
   	binds.uniforms.size = sizeof(ubinds);

   	// Bind bindings
	gs_graphics_apply_bindings(&gsi->commands, &binds);

	// Submit draw
	gs_graphics_draw_desc_t draw = gs_default_val();
	draw.start = 0; draw.count = gs_dyn_array_size(gsi->vertices);
	gs_graphics_draw(&gsi->commands, &draw);

	// Clear data
	gs_dyn_array_clear(gsi->vertices);
}

// Core pipeline functions
void gsi_blend_enabled(gs_immediate_draw_t* gsi, bool enabled)
{
	// Push a new pipeline?
	if (gsi->cache.pipeline.blend_enabled == enabled) {
		return;
	}

	// Otherwise, we need to flush previous content
	gsi_flush(gsi);

	// Set primitive type
	gsi->cache.pipeline.blend_enabled = enabled;	

	// Bind pipeline
	gs_immediate_draw_set_pipeline(gsi);
}

// Core pipeline functions
void gsi_depth_enabled(gs_immediate_draw_t* gsi, bool enabled)
{
	// Push a new pipeline?
	if (gsi->cache.pipeline.depth_enabled == (uint16_t)enabled) {
		return;
	}

	// Otherwise, we need to flush previous content
	gsi_flush(gsi);

	// Set primitive type
	gsi->cache.pipeline.depth_enabled = (uint16_t)enabled;	

	// Bind pipeline
	gs_immediate_draw_set_pipeline(gsi);
}

void gsi_stencil_enabled(gs_immediate_draw_t* gsi, bool enabled)
{
	// Push a new pipeline?
	if (gsi->cache.pipeline.stencil_enabled == (uint16_t)enabled) {
		return;
	}

	// Otherwise, we need to flush previous content
	gsi_flush(gsi);

	// Set primitive type
	gsi->cache.pipeline.stencil_enabled = (uint16_t)enabled;	

	// Bind pipeline
	gs_immediate_draw_set_pipeline(gsi);
}

void gsi_face_cull_enabled(gs_immediate_draw_t* gsi, bool enabled)
{
	// Push a new pipeline?
	if (gsi->cache.pipeline.face_cull_enabled == (uint16_t)enabled) {
		return;
	}

	// Otherwise, we need to flush previous content
	gsi_flush(gsi);

	// Set primitive type
	gsi->cache.pipeline.face_cull_enabled = (uint16_t)enabled;	

	// Bind pipeline
	gs_immediate_draw_set_pipeline(gsi);
}

void gsi_texture(gs_immediate_draw_t* gsi, gs_handle(gs_graphics_texture_t) texture)
{
	// Push a new pipeline?
	if (gsi->cache.texture.id == texture.id) {
		return;
	}

	// Otherwise, we need to flush previous content
	gsi_flush(gsi);

	// Set texture
	gsi->cache.texture = texture.id && texture.id != UINT32_MAX ? texture : GSI()->tex_default;
}

// Not working for the moment
void gsi_defaults(gs_immediate_draw_t* gsi)
{
	gsi_flush(gsi);

	// Set defaults for cache
	gsi->cache.texture = GSI()->tex_default;
	gsi->cache.pipeline = gsi_pipeline_state_default();
	gsi->cache.pipeline.prim_type = 0x00;
	gsi->cache.uv = gs_v2(0.f, 0.f);
	gsi->cache.color = GS_COLOR_WHITE;

	gs_immediate_draw_set_pipeline(gsi);
}

void gsi_tc2fv(gs_immediate_draw_t* gsi, gs_vec2 uv)
{
	// Set cache register
	gsi->cache.uv = uv;
}

void gsi_tc2f(gs_immediate_draw_t* gsi, float u, float v)
{
	// Set cache register
	gsi_tc2fv(gsi, gs_v2(u, v));
}

void gsi_c4ub(gs_immediate_draw_t* gsi, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	// Set cache color
	gsi->cache.color = gs_color(r, g, b, a);
}

void gsi_v3fv(gs_immediate_draw_t* gsi, gs_vec3 p)
{
	// Push vert
	gs_immediate_vert_t v = gs_default_val();
	v.position = p;
	v.uv = gsi->cache.uv;
	v.color = gsi->cache.color;
	gs_dyn_array_push(gsi->vertices, v);
}

void gsi_v3f(gs_immediate_draw_t* gsi, float x, float y, float z)
{
	// Push vert
	gsi_v3fv(gsi, gs_v3(x, y, z));
}

void gsi_v2f(gs_immediate_draw_t* gsi, float x, float y)
{
	// Push vert 
	gsi_v3f(gsi, x, y, 0.f);
}

void gsi_v2fv(gs_immediate_draw_t* gsi, gs_vec2 v)
{
	// Push vert 
	gsi_v3f(gsi, v.x, v.y, 0.f);
}

void gsi_push_matrix(gs_immediate_draw_t* gsi, gsi_matrix_type type)
{
	// Flush
	gsi_flush(gsi);

	// Push mode
	gs_dyn_array_push(gsi->cache.modes, type);

	// Pop matrix off of stack
	switch (gs_dyn_array_back(gsi->cache.modes))
	{
		case GSI_MATRIX_MODELVIEW: 
		{
			gs_dyn_array_push(gsi->cache.modelview, gs_dyn_array_back(gsi->cache.modelview));
		} break;	

		case GSI_MATRIX_PROJECTION: 
		{
			gs_dyn_array_push(gsi->cache.projection, gs_dyn_array_back(gsi->cache.projection));
		} break;
	}
}

void gsi_pop_matrix(gs_immediate_draw_t* gsi)
{
	// Flush
	gsi_flush(gsi);

	// Pop matrix off of stack
	switch (gs_dyn_array_back(gsi->cache.modes))
	{
		case GSI_MATRIX_MODELVIEW: {
			if (gs_dyn_array_size(gsi->cache.modelview) > 1) {
				gs_dyn_array_pop(gsi->cache.modelview);
			}
		} break;	

		case GSI_MATRIX_PROJECTION: {
			if (gs_dyn_array_size(gsi->cache.projection) > 1) {
				gs_dyn_array_pop(gsi->cache.projection);
			}
		} break;
	}

	if (gs_dyn_array_size(gsi->cache.modes) > 1) {
		gs_dyn_array_pop(gsi->cache.modes);
	}
}

void gsi_load_matrix(gs_immediate_draw_t* gsi, gs_mat4 m)
{
	// Load matrix at current mode
	switch (gs_dyn_array_back(gsi->cache.modes)) 
	{ 
		case GSI_MATRIX_MODELVIEW: {
			gs_mat4* mat = &gsi->cache.modelview[gs_dyn_array_size(gsi->cache.modelview) - 1];
			*mat = m;
		} break;

		case GSI_MATRIX_PROJECTION: {
			gs_mat4* mat = &gsi->cache.projection[gs_dyn_array_size(gsi->cache.projection) - 1];
			*mat = m;
		} break;
	}
}

void gsi_mul_matrix(gs_immediate_draw_t* gsi, gs_mat4 m)
{
	static int i = 0;
	// Multiply current matrix at mode with m
	switch (gs_dyn_array_back(gsi->cache.modes)) 
	{
		case GSI_MATRIX_MODELVIEW: {
			gs_mat4* mat = &gsi->cache.modelview[gs_dyn_array_size(gsi->cache.modelview) - 1];
			*mat = gs_mat4_mul(*mat, m);
		} break;	

		case GSI_MATRIX_PROJECTION: {
			gs_mat4* mat = &gsi->cache.projection[gs_dyn_array_size(gsi->cache.projection) - 1];
			*mat = gs_mat4_mul(*mat, m);
		} break;
	}
}

void gsi_perspective(gs_immediate_draw_t* gsi, float fov, float aspect, float n, float f)
{
	// Set current matrix at mode to perspective
	gsi_load_matrix(gsi, gs_mat4_perspective(fov, aspect, n, f));
}

void gsi_ortho(gs_immediate_draw_t* gsi, float l, float r, float b, float t, float n, float f)
{
	// Set current matrix at mode to ortho
	gsi_load_matrix(gsi, gs_mat4_ortho(l, r, b, t, n, f));
}

void gsi_rotatef(gs_immediate_draw_t* gsi, float angle, float x, float y, float z)
{
	// Rotate current matrix at mode
	gsi_mul_matrix(gsi, gs_mat4_rotatev(angle, gs_v3(x, y, z)));
}

void gsi_rotatefv(gs_immediate_draw_t* gsi, float angle, gs_vec3 v)
{
	gsi_rotatef(gsi, angle, v.x, v.y, v.z);
}

void gsi_transf(gs_immediate_draw_t* gsi, float x, float y, float z)
{
	// Translate current matrix at mode
	gsi_mul_matrix(gsi, gs_mat4_translate(x, y, z));
}

void gsi_scalef(gs_immediate_draw_t* gsi, float x, float y, float z)
{
	// Scale current matrix at mode
	gsi_mul_matrix(gsi, gs_mat4_scale(x, y, z));
}

void gsi_camera(gs_immediate_draw_t* gsi, gs_camera_t* cam)
{
	// Just grab main window for now. Will need to grab top of viewport stack in future
	gs_vec2 ws = gs_platform_window_sizev(gsi->window_handle);	
	gsi_load_matrix(gsi, gs_camera_get_view_projection(cam, (s32)ws.x, (s32)ws.y));
}

void gsi_camera2D(gs_immediate_draw_t* gsi)
{
	// Flush previous
	gsi_flush(gsi);
	gs_vec2 ws = gs_platform_window_sizev(gsi->window_handle);
	f32 l = 0.f, r = ws.x, tp = 0.f, b = ws.y;
	gs_mat4 ortho = gs_mat4_ortho(
		l, r, b, tp, -1.f, 1.f
	);
	gsi_load_matrix(gsi, ortho);
}

void gsi_camera3D(gs_immediate_draw_t* gsi)
{
	// Flush previous
	gsi_flush(gsi);
	gs_camera_t c = gs_camera_perspective();
	gsi_camera(gsi, &c);
}

// Shape Drawing Utils
void gsi_triangle(gs_immediate_draw_t* gsi, float x0, float y0, float x1, float y1, float x2, float y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type)
{
	gsi_trianglex(gsi, x0, y0, 0.f, x1, y1, 0.f, x2, y2, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, r, g, b, a, type);
}

void gsi_trianglev(gs_immediate_draw_t* gsi, gs_vec2 a, gs_vec2 b, gs_vec2 c, gs_color_t color, gs_graphics_primitive_type type)
{
	gsi_triangle(gsi, a.x, a.y, b.x, b.y, c.x, c.y, color.r, color.g, color.b, color.a, type);
}

void gsi_trianglex(gs_immediate_draw_t* gsi, float x0, float y0, float z0, float x1, float y1, float z1, float x2, 
	float y2, float z2, float u0, float v0, float u1, float v1, float u2, float v2, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type)
{
	switch (type)
	{
		default:
		case GS_GRAPHICS_PRIMITIVE_TRIANGLES:
		{
			gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
				gsi_c4ub(gsi, r, g, b, a);
				gsi_tc2f(gsi, u0, v0);
				gsi_v3f(gsi, x0, y0, z0);
				gsi_tc2f(gsi, u1, v1);
				gsi_v3f(gsi, x1, y1, z1);
				gsi_tc2f(gsi, u2, v2);
				gsi_v3f(gsi, x2, y2, z2);
			gsi_end(gsi);
		} break;

		case GS_GRAPHICS_PRIMITIVE_LINES:
		{
			gsi_line3D(gsi, x0, y0, z0, x1, y1, z1, r, g, b, a);
			gsi_line3D(gsi, x1, y1, z1, x2, y2, z2, r, g, b, a);
			gsi_line3D(gsi, x2, y2, z2, x0, y0, z0, r, g, b, a);
		} break;
	}
}

void gsi_trianglevx(gs_immediate_draw_t* gsi, gs_vec3 v0, gs_vec3 v1, gs_vec3 v2, gs_vec2 uv0, gs_vec2 uv1, gs_vec2 uv2, gs_color_t color, gs_graphics_primitive_type type)
{
	gsi_trianglex(gsi, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, v2.x, 
		v2.y, v2.z, uv0.x, uv0.y, uv1.x, uv1.y, uv2.x, uv2.y, color.r, color.g, color.b, color.a, type);
}

GS_API_DECL void gsi_trianglevxmc(
	gs_immediate_draw_t* gsi, 
	gs_vec3 v0, gs_vec3 v1, gs_vec3 v2, 
	gs_vec2 uv0, gs_vec2 uv1, gs_vec2 uv2, 
	gs_color_t c0, gs_color_t c1, gs_color_t c2, 
	gs_graphics_primitive_type type
)
{
	switch (type)
	{
		default:
		case GS_GRAPHICS_PRIMITIVE_TRIANGLES:
		{
			gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

				// First vert
				gsi_c4ub(gsi, c0.r, c0.g, c0.b, c0.a);
				gsi_tc2f(gsi, uv0.x, uv0.y);
				gsi_v3f(gsi, v0.x, v0.y, v0.z);

				// Second vert
				gsi_c4ub(gsi, c1.r, c1.g, c1.b, c1.a);
				gsi_tc2f(gsi, uv1.x, uv1.y);
				gsi_v3f(gsi, v1.x, v1.y, v1.z);

				// Third vert
				gsi_c4ub(gsi, c2.r, c2.g, c2.b, c2.a);
				gsi_tc2f(gsi, uv2.x, uv2.y);
				gsi_v3f(gsi, v2.x, v2.y, v2.z);
				
			gsi_end(gsi);
		} break;

		case GS_GRAPHICS_PRIMITIVE_LINES:
		{
			gsi_line3Dmc(gsi, v0.x, v0.y, v0.z, v1.x, v1.y, v1.z, c0.r, c0.g, c0.b, c0.a, c1.r, c1.g, c1.b, c1.a);
			gsi_line3Dmc(gsi, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, c1.r, c1.g, c1.b, c1.a, c2.r, c2.g, c2.b, c2.a);
			gsi_line3Dmc(gsi, v2.x, v2.y, v2.z, v0.x, v0.y, v0.z, c2.r, c2.g, c2.b, c2.a, c0.r, c0.g, c0.b, c0.a);
		} break;
	}
}

void gsi_line3Dmc(
	gs_immediate_draw_t* gsi, 
	float x0, float y0, float z0, 
	float x1, float y1, float z1, 
	uint8_t r0, uint8_t g0, uint8_t b0, uint8_t a0, 
	uint8_t r1, uint8_t g1, uint8_t b1, uint8_t a1 
)
{
	gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_LINES);

		gsi_tc2f(gsi, 0.f, 0.f);

		// First vert
		gsi_c4ub(gsi, r0, g0, b0, a0);
		gsi_v3f(gsi, x0, y0, z0);

		// Second vert
		gsi_c4ub(gsi, r1, g1, b1, a1);
		gsi_v3f(gsi, x1, y1, z1);
	gsi_end(gsi);
}

void gsi_line3D(gs_immediate_draw_t* gsi, float x0, float y0, float z0, float x1, float y1, float z1, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_LINES);
		gsi_tc2f(gsi, 0.f, 0.f);
		gsi_c4ub(gsi, r, g, b, a);
		gsi_v3f(gsi, x0, y0, z0);
		gsi_v3f(gsi, x1, y1, z1);
	gsi_end(gsi);
}

void gsi_line3Dv(gs_immediate_draw_t* gsi, gs_vec3 s, gs_vec3 e, gs_color_t color)
{
	gsi_line3D(gsi, s.x, s.y, s.z, e.x, e.y, e.z, color.r, color.g, color.b, color.a);
}

void gsi_line(gs_immediate_draw_t* gsi, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	gsi_line3D(gsi, x0, y0, 0.f, x1, y1, 0.f, r, g, b, a);
}

void gsi_linev(gs_immediate_draw_t* gsi, gs_vec2 v0, gs_vec2 v1, gs_color_t c)
{
	gsi_line(gsi, v0.x, v0.y, v1.x, v1.y, c.r, c.g, c.b, c.a);	
}

void gsi_rectx(gs_immediate_draw_t* gsi, float l, float b, float r, float t, float u0, float v0, float u1, float v1, uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a, gs_graphics_primitive_type type)
{
	// Shouldn't use triangles, because need to declare texture coordinates.
	switch (type)
	{
		case GS_GRAPHICS_PRIMITIVE_LINES:
		{
			// First triangle
			gsi_line(gsi, l, b, r, b, _r, _g, _b, _a);
			gsi_line(gsi, r, b, r, t, _r, _g, _b, _a);
			gsi_line(gsi, r, t, l, t, _r, _g, _b, _a);
			gsi_line(gsi, l, t, l, b, _r, _g, _b, _a);
			// gsi_triangle(gsi, l, b, r, b, l, t, _r, _g, _b, _a, type);
			// Second triangle
			// gsi_triangle(gsi, r, b, r, t, l, t, _r, _g, _b, _a, type);
		} break;

		case GS_GRAPHICS_PRIMITIVE_TRIANGLES:
		{
			gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

				gsi_c4ub(gsi, _r, _g, _b, _a);

				// First triangle
				gsi_tc2f(gsi, u0, v0); gsi_v2f(gsi, l, b);
				gsi_tc2f(gsi, u1, v0); gsi_v2f(gsi, r, b);
				gsi_tc2f(gsi, u0, v1); gsi_v2f(gsi, l, t);

				// Second triangle
				gsi_tc2f(gsi, u1, v0); gsi_v2f(gsi, r, b);
				gsi_tc2f(gsi, u1, v1); gsi_v2f(gsi, r, t);
				gsi_tc2f(gsi, u0, v1); gsi_v2f(gsi, l, t);
				
			gsi_end(gsi);

		} break;
	}
}

void gsi_rect(gs_immediate_draw_t* gsi, float l, float b, float r, float t, uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a, gs_graphics_primitive_type type)
{
	gsi_rectx(gsi, l, b, r, t, 0.f, 0.f, 1.f, 1.f, _r, _g, _b, _a, type);
}

void gsi_rectv(gs_immediate_draw_t* gsi, gs_vec2 bl, gs_vec2 tr, gs_color_t color, gs_graphics_primitive_type type)
{
	gsi_rectx(gsi, bl.x, bl.y, tr.x, tr.y, 0.f, 0.f, 1.f, 1.f, color.r, color.g, color.b, color.a, type);
}

void gsi_rectvx(gs_immediate_draw_t* gsi, gs_vec2 bl, gs_vec2 tr, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color, gs_graphics_primitive_type type)
{
	gsi_rectx(gsi, bl.x, bl.y, tr.x, tr.y, uv0.x, uv0.y, uv1.x, uv1.y, color.r, color.g, color.b, color.a, type);
}

void gsi_rectvd(gs_immediate_draw_t* gsi, gs_vec2 xy, gs_vec2 wh, gs_vec2 uv0, gs_vec2 uv1, gs_color_t color, gs_graphics_primitive_type type)
{
	gsi_rectx(gsi, xy.x, xy.y, xy.x + wh.x, xy.y + wh.y, uv0.x, uv0.y, uv1.x, uv1.y, color.r, color.g, color.b, color.a, type);
}

GS_API_DECL void gsi_rect3Dv(gs_immediate_draw_t* gsi, gs_vec3 min, gs_vec3 max, gs_vec2 uv0, gs_vec2 uv1, gs_color_t c, gs_graphics_primitive_type type)
{
	const gs_vec3 vt0 = min;
	const gs_vec3 vt1 = gs_v3(max.x, min.y, min.z);
	const gs_vec3 vt2 = gs_v3(min.x, max.y, max.z);
	const gs_vec3 vt3 = max;

	switch (type)
	{
		case GS_GRAPHICS_PRIMITIVE_LINES:
		{
			gsi_line3Dv(gsi, vt0, vt1, c);
			gsi_line3Dv(gsi, vt1, vt2, c);
			gsi_line3Dv(gsi, vt2, vt3, c);
			gsi_line3Dv(gsi, vt3, vt0, c);
		} break;

		case GS_GRAPHICS_PRIMITIVE_TRIANGLES:
		{
			gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

				gsi_c4ub(gsi, c.r, c.g, c.b, c.a);

				const float u0 = uv0.x;
				const float u1 = uv1.x;
				const float v0 = uv0.y;
				const float v1 = uv1.y;

				// First triangle
				gsi_c4ub(gsi, c.r, c.g, c.b, c.a);
				gsi_tc2f(gsi, u0, v0); gsi_v3fv(gsi, vt0);
				gsi_tc2f(gsi, u1, v0); gsi_v3fv(gsi, vt3);
				gsi_tc2f(gsi, u0, v1); gsi_v3fv(gsi, vt1);

				// Second triangle
				gsi_c4ub(gsi, c.r, c.g, c.b, c.a);
				gsi_tc2f(gsi, u1, v0); gsi_v3fv(gsi, vt0);
				gsi_tc2f(gsi, u1, v1); gsi_v3fv(gsi, vt2);
				gsi_tc2f(gsi, u0, v1); gsi_v3fv(gsi, vt3);
				
			gsi_end(gsi);

		} break;
	}
}

void gsi_circle_sector(gs_immediate_draw_t* gsi, float cx, float cy, float radius, int32_t start_angle, 
	int32_t end_angle, int32_t segments, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type)
{
    if (radius <= 0.0f) {
    	radius = 0.1f;
    }

    // Function expects (end_angle > start_angle)
    if (end_angle < start_angle) {
        // Swap values
        int32_t tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4) {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - gsi_smooth_circle_error_rate/radius, 2) - 1);
        segments = (int32_t)((end_angle - start_angle)*ceilf(2*GS_PI/th)/360);
        if (segments <= 0) {
        	segments = 4;
        }
    }

    float step = (float)(end_angle - start_angle)/(float)segments;
    float angle = (float)start_angle;
	gs_for_range_i(segments)
    {
        gs_vec2 _a = gs_v2(cx, cy);
        gs_vec2 _b = gs_v2(cx + sinf(gsi_deg2rad*angle)*radius, cy + cosf(gsi_deg2rad*angle)*radius);
        gs_vec2 _c = gs_v2(cx + sinf(gsi_deg2rad*(angle + step))*radius, cy + cosf(gsi_deg2rad*(angle + step))*radius);
		gsi_trianglev(gsi, _a, _b, _c, gs_color(r, g, b, a), type);
        angle += step;
    }
}

void gsi_circle_sectorvx(gs_immediate_draw_t* gsi, gs_vec3 c, float radius, int32_t start_angle, 
	int32_t end_angle, int32_t segments, gs_color_t color, gs_graphics_primitive_type type)
{
    if (radius <= 0.0f) {
    	radius = 0.1f;
    }

    // Cache elements of center vector
    float cx = c.x, cy = c.y, cz = c.z;

    // Function expects (end_angle > start_angle)
    if (end_angle < start_angle) {
        // Swap values
        int32_t tmp = start_angle;
        start_angle = end_angle;
        end_angle = tmp;
    }

    if (segments < 4) {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - gsi_smooth_circle_error_rate/radius, 2) - 1);
        segments = (int32_t)((end_angle - start_angle)*ceilf(2*GS_PI/th)/360);
        if (segments <= 0) {
        	segments = 4;
        }
    }

    float step = (float)(end_angle - start_angle)/(float)segments;
    float angle = (float)start_angle;
	gs_for_range_i(segments)
    {
        gs_vec3 _a = gs_v3(cx, cy, cz);
        gs_vec3 _b = gs_v3(cx + sinf(gsi_deg2rad*angle)*radius, cy + cosf(gsi_deg2rad*angle)*radius, cz);
        gs_vec3 _c = gs_v3(cx + sinf(gsi_deg2rad*(angle + step))*radius, cy + cosf(gsi_deg2rad*(angle + step))*radius, cz);
		gsi_trianglevx(gsi, _a, _b, _c, gs_v2s(0.f), gs_v2s(0.5f), gs_v2s(1.f), color, type);
        angle += step;
    }
}

void gsi_circle(gs_immediate_draw_t* gsi, float cx, float cy, float radius, int32_t segments, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type)
{
	gsi_circle_sector(gsi, cx, cy, radius, 0, 360, segments, r, g, b, a, type);	
}

void gsi_circlevx(gs_immediate_draw_t* gsi, gs_vec3 c, float radius, int32_t segments, gs_color_t color, gs_graphics_primitive_type type)
{
	gsi_circle_sectorvx(gsi, c, radius, 0, 360, segments, color, type);
}

void gsi_box(gs_immediate_draw_t* gsi, float x, float y, float z, float hx, float hy, float hz, 
		uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type)
{
	float width = hx;
	float height = hy;
	float length = hz;

	gs_vec3 v0 = gs_v3(x - width, y - height, z + length);
	gs_vec3 v1 = gs_v3(x + width, y - height, z + length);
	gs_vec3 v2 = gs_v3(x - width, y + height, z + length);
	gs_vec3 v3 = gs_v3(x + width, y + height, z + length);
	gs_vec3 v4 = gs_v3(x - width, y - height, z - length);
	gs_vec3 v5 = gs_v3(x - width, y + height, z - length);
	gs_vec3 v6 = gs_v3(x + width, y - height, z - length);
	gs_vec3 v7 = gs_v3(x + width, y + height, z - length);

	gs_color_t color = gs_color(r, g, b, a);

	switch (type)
	{
		case GS_GRAPHICS_PRIMITIVE_TRIANGLES:
		{
			gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
			{
				gs_vec2 uv0 = gs_v2(0.f, 0.f);
				gs_vec2 uv1 = gs_v2(1.f, 0.f);
				gs_vec2 uv2 = gs_v2(0.f, 1.f);
				gs_vec2 uv3 = gs_v2(1.f, 1.f);

		        // Front Face
		        gsi_trianglevx(gsi, v0, v1, v2, uv0, uv1, uv2, color, type);
		        gsi_trianglevx(gsi, v3, v2, v1, uv3, uv2, uv1, color, type);
		        
		        // Back face
		        gsi_trianglevx(gsi, v6, v5, v7, uv0, uv3, uv2, color, type);
		        gsi_trianglevx(gsi, v6, v4, v5, uv0, uv1, uv3, color, type);

		        // Top face
		        gsi_trianglevx(gsi, v7, v2, v3, uv0, uv3, uv2, color, type);
		        gsi_trianglevx(gsi, v7, v5, v2, uv0, uv1, uv3, color, type);

		        // Bottom face
		        gsi_trianglevx(gsi, v4, v1, v0, uv0, uv3, uv2, color, type);
		        gsi_trianglevx(gsi, v4, v6, v1, uv0, uv1, uv3, color, type);

		        // Right face
		        gsi_trianglevx(gsi, v1, v7, v3, uv0, uv3, uv2, color, type);
		        gsi_trianglevx(gsi, v1, v6, v7, uv0, uv1, uv3, color, type);

		        // Left face
		        gsi_trianglevx(gsi, v4, v2, v5, uv0, uv3, uv2, color, type);
		        gsi_trianglevx(gsi, v4, v0, v2, uv0, uv1, uv3, color, type);
			}
			gsi_end(gsi);
		} break;

		case GS_GRAPHICS_PRIMITIVE_LINES:
		{
			gs_color_t color = gs_color(r, g, b, a);
			gsi_tc2f(gsi, 0.f, 0.f);

			// Front face
			gsi_line3Dv(gsi, v0, v1, color);
			gsi_line3Dv(gsi, v1, v3, color);
			gsi_line3Dv(gsi, v3, v2, color);
			gsi_line3Dv(gsi, v2, v0, color);

				// Back face
			gsi_line3Dv(gsi, v4, v6, color);
			gsi_line3Dv(gsi, v6, v7, color);
			gsi_line3Dv(gsi, v7, v5, color);
			gsi_line3Dv(gsi, v5, v4, color);

				// Right face
			gsi_line3Dv(gsi, v1, v6, color);
			gsi_line3Dv(gsi, v6, v7, color);
			gsi_line3Dv(gsi, v7, v3, color);
			gsi_line3Dv(gsi, v3, v1, color);

				// Left face
			gsi_line3Dv(gsi, v4, v6, color);
			gsi_line3Dv(gsi, v6, v1, color);
			gsi_line3Dv(gsi, v1, v0, color);
			gsi_line3Dv(gsi, v0, v4, color);

				// Bottom face
			gsi_line3Dv(gsi, v5, v7, color);
			gsi_line3Dv(gsi, v7, v3, color);
			gsi_line3Dv(gsi, v3, v2, color);
			gsi_line3Dv(gsi, v2, v5, color);

				// Top face
			gsi_line3Dv(gsi, v0, v4, color);
			gsi_line3Dv(gsi, v4, v5, color);
			gsi_line3Dv(gsi, v5, v2, color);
			gsi_line3Dv(gsi, v2, v0, color);

		} break;
	}
}

void gsi_sphere(gs_immediate_draw_t* gsi, float cx, float cy, float cz, float radius, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type)
{
	// Modified from: http://www.songho.ca/opengl/gl_sphere.html
	const uint32_t stacks = 16;
	const uint32_t sectors = 32; 
	float sector_step = 2.f * (float)GS_PI / (float)sectors;
	float stack_step = (float)GS_PI / (float)stacks;
	struct { 
		gs_vec3 p;
		gs_vec2 uv;
	} v0, v1, v2, v3;
	gs_color_t color = gs_color(r, g, b, a);

	// TODO(john): Need to get these verts to be positioned correctly (translate then rotate all verts to correct for odd 90 degree rotation)
	#define make_vert(V, I, J, XZ, SECANGLE)\
		do {\
	        /* vertex position (x, y, z) */\
		    V.p.x = cx + (XZ) * cosf((SECANGLE));\
		    V.p.z = cz + (XZ) * sinf((SECANGLE));\
	        /* vertex tex coord (s, t) range between [0, 1] */\
	        V.uv.x = (float)(J) / sectors;\
	        V.uv.y = (float)(I) / stacks;\
		} while (0)

	#define push_vert(V)\
		do {\
	        gsi_tc2f(gsi, V.s, V.t);\
	        gsi_v3f(gsi, V.x, V.y, V.z);\
		} while (0)

	for (uint32_t i = 0; i < stacks; ++i)
	{
		float sa0 = GS_PI / 2.f - i * stack_step;      
	    float sa1 = GS_PI / 2.f - (i + 1) * stack_step;
	    float xz0 = radius * cosf(sa0); 
	    float xz1 = radius * cosf(sa1); 
	    float y0 = cy + radius * sinf(sa0);            // r * sin(u)
	    float y1 = cy + radius * sinf(sa1);            // r * sin(u)

	    v0.p.y = y0;
	    v1.p.y = y0;
	    v2.p.y = y1;
	    v3.p.y = y1;

	    for(uint32_t j = 0; j < sectors; ++j)
	    {
	        float sca0 = j * sector_step;           // starting from 0 to 2pi
	        float sca1 = (j + 1) * sector_step;

	        // Make verts
	        make_vert(v0, i, j, xz0, sca0);
	        make_vert(v1, i, j + 1, xz0, sca1);
	        make_vert(v2, i + 1, j, xz1, sca0);
	        make_vert(v3, i + 1, j + 1, xz1, sca1);

	        // First triangle
	        gsi_trianglevx(gsi, v0.p, v3.p, v2.p, v0.uv, v3.uv, v2.uv, color, type);
	        // Second triangle
	        gsi_trianglevx(gsi, v0.p, v1.p, v3.p, v0.uv, v1.uv, v3.uv, color, type);
	    }
	}
}

// Modified from Raylib's implementation
void gsi_bezier(gs_immediate_draw_t* gsi, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	gs_vec2 start = gs_v2(x0, y0);
	gs_vec2 end = gs_v2(x1, y1);
	gs_vec2 previous = start;
    gs_vec2 current = gs_default_val();
    gs_color_t color = gs_color(r, g, b, a);
    const uint32_t bezier_line_divisions = 24;

    for (int i = 1; i <= bezier_line_divisions; i++)
    {
        current.y = gs_ease_cubic_in_out((float)i, start.y, end.y - start.y, (float)bezier_line_divisions);
        current.x = previous.x + (end.x - start.x)/ (float)bezier_line_divisions;
        gsi_linev(gsi, previous, current, color);
        previous = current;
    }	
}

GS_API_DECL void gsi_cylinder(gs_immediate_draw_t* gsi, float x, float y, float z, float r_top, 
	float r_bottom, float height, int32_t sides, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type)
{
    if (sides < 3) sides = 3;

    int32_t numVertex = sides * 8;
    const float hh = height * 0.5f;

    switch (type)
    {
    	default:
    	case GS_GRAPHICS_PRIMITIVE_TRIANGLES:
    	{
    		gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    		{
				gsi_c4ub(gsi, r, g, b, a);

				if (sides < 3) sides = 3;

			    numVertex = sides * 6;

	            if (r_top > 0)
	            {
	                // Draw Body -------------------------------------------------------------------------------------
	                for (int i = 0; i < 360; i += 360/sides)
	                {
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_bottom, y - hh, z + cosf(gsi_deg2rad*i)*r_bottom); //Bottom Left
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom, y - hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom); //Bottom Right
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_top, y + hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_top); //Top Right

	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_top, y + hh, z + cosf(gsi_deg2rad*i)*r_top); //Top Left
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_bottom, y - hh, z + cosf(gsi_deg2rad*i)*r_bottom); //Bottom Left
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_top, y + hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_top); //Top Right
	                }

	                // Draw Cap --------------------------------------------------------------------------------------
	                for (int i = 0; i < 360; i += 360/sides)
	                {
	                    gsi_v3f(gsi, x + 0, y + hh, z + 0);
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_top, y + hh, z + cosf(gsi_deg2rad*i)*r_top);
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_top, y + hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_top);
	                }
	            }
	            else
	            {
	                // Draw Cone -------------------------------------------------------------------------------------
	                for (int i = 0; i < 360; i += 360/sides)
	                {
	                    gsi_v3f(gsi, x + 0, y + hh, z + 0);
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_bottom, y - hh, z + cosf(gsi_deg2rad*i)*r_bottom);
	                    gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom, y - hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom);
	                }
	            }

	            // Draw Base -----------------------------------------------------------------------------------------
	            for (int i = 0; i < 360; i += 360/sides)
	            {
	                gsi_v3f(gsi, x + 0, y - hh, z + 0);
	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom, y - hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom);
	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_bottom, y - hh, z + cosf(gsi_deg2rad*i)*r_bottom);
	            }
    		}
    		gsi_end(gsi);
    	} break;

    	case GS_GRAPHICS_PRIMITIVE_LINES: 
    	{
    		gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_LINES);
    		{
				gsi_c4ub(gsi, r, g, b, a);

	            for (int32_t i = 0; i < 360; i += 360/sides)
	            {
	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_bottom, y - hh, cosf(gsi_deg2rad*i)*r_bottom);
	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom, y - hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom);

	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom, y - hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom);
	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_top, y + hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_top);

	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_top, y + hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_top);
	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_top, y + hh, z + cosf(gsi_deg2rad*i)*r_top);

	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_top, y + hh, z + cosf(gsi_deg2rad*i)*r_top);
	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*i)*r_bottom, y - hh, z + cosf(gsi_deg2rad*i)*r_bottom);
	            }

	            // Draw Top/Bottom circles
	            for (int i = 0; i < 360; i += 360/sides)
	            {
	                gsi_v3f(gsi, x, y - hh, z);
	                gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom, y - hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_bottom);

	                if (r_top) {
		                gsi_v3f(gsi, x + 0, y + hh, z + 0);
		                gsi_v3f(gsi, x + sinf(gsi_deg2rad*(i + 360.0f/sides))*r_top, y + hh, z + cosf(gsi_deg2rad*(i + 360.0f/sides))*r_top);
	                }
	            }
    		}
    		gsi_end(gsi);

    	} break;
    }
}

GS_API_DECL void gsi_cone(gs_immediate_draw_t* gsi, float x, float y, float z, float radius, float height, int32_t sides, uint8_t r, uint8_t g, uint8_t b, uint8_t a, gs_graphics_primitive_type type)
{
	gsi_cylinder(gsi, x, y, z, 0.f, radius, height, sides, r, g, b, a, type);
}

void gsi_text(gs_immediate_draw_t* gsi, float x, float y, const char* text, const gs_asset_font_t* fp, bool32_t flip_vertical, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	// If no font, set to default
	if (!fp) {
		fp = &GSI()->font_default;
	}

	gsi_texture(gsi, fp->texture.hndl);

	gs_mat4 rot = gs_mat4_rotatev(gs_deg2rad(-180.f), GS_XAXIS);

    // Get total dimensions of text
    gs_vec2 td = gs_asset_font_text_dimensions(fp, text, -1);
    float th = gs_asset_font_max_height(fp);
    
    // Move text to accomdate height
    // y += td.y;
    y += th;

	// Needs to be fixed in here. Not elsewhere.
	gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
	{
		gsi_c4ub(gsi, r, g, b, a);
		while (text[0] != '\0')
		{
			char c = text[0];
			if (c >= 32 && c <= 127) 
			{
				stbtt_aligned_quad q = gs_default_val();
				stbtt_GetBakedQuad((stbtt_bakedchar*)fp->glyphs, fp->texture.desc.width, fp->texture.desc.height, c - 32, &x, &y, &q, 1);

				gs_vec3 v0 = gs_v3(q.x0, q.y0, 0.f);	// TL
				gs_vec3 v1 = gs_v3(q.x1, q.y0, 0.f);	// TR
				gs_vec3 v2 = gs_v3(q.x0, q.y1, 0.f);	// BL
				gs_vec3 v3 = gs_v3(q.x1, q.y1, 0.f);	// BR

				if (flip_vertical) {
					v0 = gs_mat4_mul_vec3(rot, v0);
					v1 = gs_mat4_mul_vec3(rot, v1);
					v2 = gs_mat4_mul_vec3(rot, v2);
					v3 = gs_mat4_mul_vec3(rot, v3);
				}

				gs_vec2 uv0 = gs_v2(q.s0, q.t0);	// TL
				gs_vec2 uv1 = gs_v2(q.s1, q.t0);	// TR
				gs_vec2 uv2 = gs_v2(q.s0, q.t1);	// BL
				gs_vec2 uv3 = gs_v2(q.s1, q.t1);	// BR

				gsi_tc2fv(gsi, uv0);
				gsi_v3fv(gsi, v0);

		        gsi_tc2fv(gsi, uv3); 
				gsi_v3fv(gsi, v3); 

		        gsi_tc2fv(gsi, uv2); 
				gsi_v3fv(gsi, v2); 

				gsi_tc2fv(gsi, uv0); 
				gsi_v3fv(gsi, v0);

		        gsi_tc2fv(gsi, uv1); 
				gsi_v3fv(gsi, v1); 

		        gsi_tc2fv(gsi, uv3); 
				gsi_v3fv(gsi, v3); 
			}
			text++;
		}
	}
	gsi_end(gsi);
}

// View/Scissor commands
GS_API_DECL void gsi_set_view_scissor(gs_immediate_draw_t* gsi, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    // Flush previous
    gsi_flush(gsi);

    // Set graphics viewport scissor
    gs_graphics_set_view_scissor(&gsi->commands, x, y, w, h); 
}

// Final Submit / Merge
void gsi_draw(gs_immediate_draw_t* gsi, gs_command_buffer_t* cb)
{
	// Final flush (if necessary)(this might be a part of gsi_end() instead)
	gsi_flush(gsi);

	// Merge gsi commands to end of cb
	gs_byte_buffer_write_bulk(&cb->commands, gsi->commands.commands.data, gsi->commands.commands.position);

	// Increase number of commands of merged buffer
	cb->num_commands += gsi->commands.num_commands;

	// Reset cache
	gsi_reset(gsi);
}

void gsi_render_pass_submit(gs_immediate_draw_t* gsi, gs_command_buffer_t* cb, gs_color_t c)
{
	gs_graphics_clear_action_t action = gs_default_val();
	action.color[0] = (float)c.r / 255.f; 
	action.color[1] = (float)c.g / 255.f; 
	action.color[2] = (float)c.b / 255.f; 
	action.color[3] = (float)c.a / 255.f;
	gs_graphics_clear_desc_t clear = gs_default_val();
	clear.actions = &action;
	gs_renderpass pass = gs_default_val();
	gs_vec2 fb = gs_platform_framebuffer_sizev(gsi->window_handle);
	gs_graphics_begin_render_pass(cb, pass);
	gs_graphics_set_viewport(cb, 0, 0, (int32_t)fb.x, (int32_t)fb.y);
	gs_graphics_clear(cb, &clear);
	gsi_draw(gsi, cb);
	gs_graphics_end_render_pass(cb);
}

GS_API_DECL void gsi_render_pass_submit_ex(gs_immediate_draw_t* gsi, gs_command_buffer_t* cb, gs_graphics_clear_action_t* action)
{
    gs_graphics_clear_desc_t clear = gs_default_val();
    clear.actions = action;
	gs_renderpass pass = gs_default_val();
	gs_vec2 fb = gs_platform_framebuffer_sizev(gsi->window_handle);
	gs_graphics_begin_render_pass(cb, pass);
	gs_graphics_set_viewport(cb, 0, 0, (int32_t)fb.x, (int32_t)fb.y);
	gs_graphics_clear(cb, &clear);
	gsi_draw(gsi, cb);
	gs_graphics_end_render_pass(cb);
}

//-----------------------------------------------------------------------------
// [SECTION] Default font data (ProggyClean.ttf)
//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using misc/fonts/binary_to_compressed_c.cpp (with compression + base85 string encoding).
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
//-----------------------------------------------------------------------------

// Modified from stb lib for embedding without collisions
GS_API_DECL unsigned int gs_decompress_length(const unsigned char* input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *gs__barrier;
static unsigned char *gs__barrier2;
static unsigned char *gs__barrier3;
static unsigned char *gs__barrier4;

static unsigned char *gs__dout;
static void gs__match(const unsigned char *data, unsigned int length)
{
   // INVERSE of memmove... write each byte before copying the next...
   assert (gs__dout + length <= gs__barrier);
   if (gs__dout + length > gs__barrier) { gs__dout += length; return; }
   if (data < gs__barrier4) { gs__dout = gs__barrier+1; return; }
   while (length--) *gs__dout++ = *data++;
}

static void gs__lit(const unsigned char *data, unsigned int length)
{
   assert (gs__dout + length <= gs__barrier);
   if (gs__dout + length > gs__barrier) { gs__dout += length; return; }
   if (data < gs__barrier2) { gs__dout = gs__barrier+1; return; }
   memcpy(gs__dout, data, length);
   gs__dout += length;
}

#define gs__in2(x)   ((i[x] << 8) + i[(x)+1])
#define gs__in3(x)   ((i[x] << 16) + gs__in2((x)+1))
#define gs__in4(x)   ((i[x] << 24) + gs__in3((x)+1))

static unsigned char *gs_decompress_token(unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       gs__match(gs__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  gs__match(gs__dout-(gs__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ gs__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       gs__match(gs__dout-(gs__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  gs__match(gs__dout-(gs__in3(0) - 0x100000 + 1), gs__in2(3)+1), i += 5;
        else if (*i >= 0x08)  gs__lit(i+2, gs__in2(0) - 0x0800 + 1), i += 2 + (gs__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  gs__lit(i+3, gs__in2(1) + 1), i += 3 + (gs__in2(1) + 1);
        else if (*i == 0x06)  gs__match(gs__dout-(gs__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  gs__match(gs__dout-(gs__in3(1)+1), gs__in2(4)+1), i += 6;
    }
    return i;
}

unsigned int gs_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen = buflen % 5552;

    unsigned long i;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

GS_API_DECL unsigned int gs_decompress(unsigned char *output, unsigned char *i, unsigned int length)
{
   uint32_t olen;
   if (gs__in4(0) != 0x57bC0000) return 0;
   if (gs__in4(4) != 0)          return 0; // error! stream is > 4GB
   olen = gs_decompress_length(i);
   gs__barrier2 = i;
   gs__barrier3 = i+length;
   gs__barrier = output + olen;
   gs__barrier4 = output;
   i += 16;

   gs__dout = output;
   while (1) {
      unsigned char *old_i = i;
      i = gs_decompress_token(i);
      if (i == old_i) {
         if (*i == 0x05 && i[1] == 0xfa) {
            assert(gs__dout == output + olen);
            if (gs__dout != output + olen) return 0;
            if (gs_adler32(1, output, olen) != (uint32_t) gs__in4(2))
               return 0;
            return olen;
         } else {
            assert(0); /* NOTREACHED */
            return 0;
         }
      }
      assert(gs__dout <= output + olen);
      if (gs__dout > output + olen)
         return 0;
   }
}

GS_API_DECL unsigned int GSDecode85Byte(char c)                                    {return c >= '\\' ? c-36 : c-35;}
GS_API_DECL void         GSDecode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = GSDecode85Byte(src[0]) + 85 * (GSDecode85Byte(src[1]) + 85 * (GSDecode85Byte(src[2]) + 85 * (GSDecode85Byte(src[3]) + 85 * GSDecode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

static const char __gs_proggy_clean_ttf_compressed_data_base85[11980 + 1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

GS_API_DECL const char* GSGetDefaultCompressedFontDataTTFBase85()
{
    return __gs_proggy_clean_ttf_compressed_data_base85;
}

#undef GS_IMMEDIATE_DRAW_IMPL
#endif // GS_IMMEDIATE_DRAW_IMPL

/* 
	Sketches:

	// You can push/pop pipelines?
	// Push/pop vertex data

	typedef struct gs_immediate_draw_i
	{
		gs_command_buffer_t cb;
		...
	} gs_immediate_draw_i;

	// Each include "shape" will have certain vertex attributes that it can upload?
	// So you could render something with a different pipeline?

	// Create immediate mode (just opengl style immediate mode stuff).
	// Create util on top of this to allow for custom shape drawing into immediate mode.

	// Could do this instead? Push flags onto stack, then ANY provided function called will use this flag set instead?
	// Just have to provide functionality in draw functions to allow this.
	push_vattr_flags(attr_flags*, size_t sz);

	// If you call this, it will push the default "fill" pipeline on
	// layout = [FLOAT_3, FLOAT_2, FLOAT_3, UINT8_4] // position, uv, normal, color
	gsi_triangle();

	// If you call this, or any other line function, it will push the default "line" pipeline on.
	// layout = [FLOAT_3, UINT8_4] // position, color
	gsi_line();

	// If you call this function, the pipeline won't change. It will instead push back interleaved data (in order provided) of attributes provided.
	// Can use for custom pipelines.
	gsi_line_vattr(..., attr_flags*, size_t attr_flags_sz);

	gsi_vattr_flags flags[] = {
		GS_IVATTR_POSITION,	-> Provided vattr for all shapes
		GS_IVATTR_NORMAL,	-> Provided vattr for all shapes
		GS_IVATTR_UV,		-> Provided vattr for all shapes
		GS_IVATTR_COLOR,	-> Provided vattr for all shapes
		GS_IVATTR_FLOAT4,	-> General float4 (custom)
		GS_IVATTR_FLOAT3,	-> General float3 (custom)
		GS_IVATTR_FLOAT2,	-> General float2 (custom)
		GS_IVATTR_FLOAT,	-> General float  (custom)
		GS_IVATTR_UINT4,	-> General uint4  (custom)
		GS_IVATTR_UINT3,	-> General uint3  (custom)
		GS_IVATTR_UINT2,	-> General uint2  (custom)
		GS_IVATTR_UINT,	 	-> General uint   (custom)
		GS_IVATTR_BYTE4,	-> General byte4  (custom)
		GS_IVATTR_BYTE3,	-> General byte3  (custom)
		GS_IVATTR_BYTE2,	-> General byte2  (custom)
		GS_IVATTR_BYTE,		-> General byte   (custom)
	};

	// This really should be outside the scope of the immediate draw and more into a specific "immediate_shapes" area. 
	// Don't want this to be too fucking cheesy though...
	// Also, it's not really supposed to take care of EVERYTHING. That's not the point.

	// Don't want to create index/vertex buffers. That's really it.
	// Just write vertex data to buffer.

	uniform buffer handles, uniform data
	sampler buffer handles, sampler handle, texture handle

	gs_immediate_draw_i* gsi = ...;

	gsi_push_pipeline(gsi);

		gsi_load_pipeline(gsi, pipeline);
		gsi_push_binds(gsi);

			gsi_load_uniforms(gsi, hndl, uniform_data);
			gsi_load_samplers(gsi, hndl, texture_handle);

			size_t coffset = (3 * sizeof(f32) + 2 * sizeof(f32));
			gsi_interleaveub(gsi, coffset, 4, r, g, b, a);

			// v0
			gsi_v3f(gsi, ...);
			gsi_v2f(gsi, ...);

			// v1
			gsi_v3f(gsi, ...);
			gsi_v2f(gsi, ...);

			// v2
			gsi_v3f(gsi, ...);
			gsi_v2f(gsi, ...);

			// Push indices as well
			gsi_idxli(gsi, 6, 0, 3, 2, 0, 1, 3);

		gsi_pop_binds(gsi);

	gsi_pop_pipeline(gsi);

	// Push/pop matrix would be useless in this - unless you bound that information for your shader.
	// What kicks off an actual submission in this case?

	gsi_matrixmode(GSI_MODELVIEW);    // To operate on the model-view matrix
    gsi_loadidentity();              // Reset model-view matrix
 
	gsi_translatef(ballX, ballY, 0.0f);  // Translate to (xPos, yPos)
	// Use triangular segments to form a circle
	gsi_begin(GS_PRIMITIVE_TRIANGLE_FAN);
	gsi_c3f(0.0f, 0.0f, 1.0f);  // Blue
	gsi_v2f(0.0f, 0.0f);       // Center of circle
	int32_t numSegments = 100;
	float angle;
	for (int32_t i = 0; i <= numSegments; i++) { // Last vertex same as first vertex
	    angle = i * 2.0f * GS_PI / numSegments;  // 360 deg for all segments
	    gs_v2f(cos(angle) * ballRadius, sin(angle) * ballRadius);
	}
	gsi_end();

	// Custom stuff?
	gsi_push_pipeline(custom);
		push_matrix(model);
			uniform_data.mvp = gs_mat_mul(gsi_matrix(vp), gsi_matrix(model));
			gsi_load_uniforms(gsi, hndl, uniform_data);
			gsi_load_samplers(gsi, hndl, texture_handle);
			size_t coffset = (3 * sizeof(f32) + 2 * sizeof(f32));
			gsi_interleaveub(gsi, coffset, 4, r, g, b, a);
			gsi_v3f(gsi, ...);
			gsi_v2f(gsi, ...);
			gsi_v3f(gsi, ...);
			gsi_v2f(gsi, ...);
			gsi_v3f(gsi, ...);
			gsi_v2f(gsi, ...);
			// Push indices as well
			gsi_idxli(gsi, 6, 0, 3, 2, 0, 1, 3);
		pop_matrix(model);
	gsi_pop_pipeline();

	gsdbg_push_default_pipeline(type)
	{
		if (type == dbg.cur_type){
			nothing;
		}
	}

	gsi_idxlist();

	gsi_begin(GS_PRIMITIVE_LINES);
		gsi_c4ub(r, g, b, a);
		gsi_tc2f(uv);
		gsi_v3f(v0);
		gsi_v3f(v1);
	gsi_end();

	gsi_begin(GS_PRIMITIVE_TRIANGLES)		-> looks for a particular default pipeline in the back
		gsi_c4ub(r, g, b, a)				-> sets color
		gsi_tc2f(u, v)						-> sets uv coordinate
		gsi_v3f()							-> pushes vert
		gsi_enable_texture();				-> 
		gsi_load_texture(tex);				-> 
	gsi_end()

	// Line triangle
	gsi_push_matrix(model);
		gsi_begin(lines);
			gsi_c4ub(r, g, b, a);
			gsi_v3f(0, 0, 0);
			gsi_v3f(1, 0, 0);
			gsi_v3f(1, 1, 0);
		gsi_end();
	gsi_pop_matrix();

	void gsdbg_triangle(a, b, c, r, g, b, a)
	{
		gsi_begin(prim_type);
		{
			gsi_c4ub(r, g, b, a);
			gsi_v3f(0, 0, 0);
			gsi_v3f(1, 0, 0);
			gsi_v3f(1, 1, 0);
		}
		gsi_end();
	}

	void gsdbg_triangle(a, b, c, color)
	{
		gsdbg_push_default_pipeline(FILL);
		size_t coffset = (3 * sizeof(f32) + 2 * sizeof(f32));
		gsi_interleaveub(gsi, coffset, 4, r, g, b, a);
		// v0
		gsi_v3f(gsi, ...);
		gsi_v2f(gsi, ...);
		// v1
		gsi_v3f(gsi, ...);
		gsi_v2f(gsi, ...);
		// v2
		gsi_v3f(gsi, ...);
		gsi_v2f(gsi, ...);
		// Push indices
		gsi_idxli(gsi, 3, 0, 1, 2);
	}

	gsi_push_binds();
	gsi_push_matrix(view_proj);
		gsi_mat_mul(vp);			-> view_projection matrix

	gsi_push_matrix(model);
		gsi_interleaveub(offset, 4, r, g, b, a); -> color

		gsi_mat_transf();	-> model matrix
		gsi_mat_rotatef();
		gsi_mat_scalef();

		gsi_v3f(x, y, z);	 -> multiplies by model/vp (has to be 3f or 4f)
		gsi_v4f(x, y, z, w);

		gsi_3f(...);
		gsi_4f(...);

	// Load 'lines' pipeline
	gsi_begin(lines);
		gsi_push_matrix(model);
			gsi_mat_mul_vqs();
			gsdbg_trianglelines();
		gsi_pop_matrix();
		gsi_push_matrix(model);
			gsi_mat_mul_vqs();
			gsdb_rectlines();
		gsi_pop_matrix();
	gsi_end();

	// Will load "triangles" pipeline and render debug text, setting texture to be used
	gsdbg_text(...);

	// Immediate
	gsi_v2f();
	gsi_v3f();
	gsi_n3f();

	// Debug shape drawing
	gsi_trianglelines();
	gsi_triangle();
	gsi_text();
	gsi_box();
	gsi_boxlines();
	gsi_circle();
	gsi_circlelines();
	gsi_rect();
	gsi_rectlines();

	gsi_begin(pipeline);
		gsi_push_matrix();
			gsi_mat_rotatef();
			gsi_mat_transf();
			gsi_mat_scalef();
			gsi_interleavef(0, 1, 2 * sizeof(f32));		-> will push back data to be interleaved with ALL vertex data
			gsi_2f(0, 1);
			gsi_2f(1, 0);
			gsi_2f(1, 1);
		gsi_pop_matrix();

		// Binding uniforms (if at all)
		// Lighting Vertices
		// Materials (specularity, diffuse, abmience)

		gsi_clearall();				 -> clears interleaved data from backend
	gsi_end();

	// Triangle specification
	gsi_begin(pipe);			// Default pipeline? Is there a default pipeline provided?
		gsi_2f(0, 0);
		gsi_4f(1, 1, 1, 1);
		gsi_2f(1, 0);
		gsi_4f(1, 1, 1, 1);
		gsi_2f(1, 1);
		gsi_4f(1, 1, 1, 1);
	gsi_end();

	// I don't like pushing/popping matrices, really...but I should do that.
	// Kind of goes against my idea of NOT having to keep this stuff internal.

	gsdbg_triangle();
	gsdbg_cube();

	gl_begin(prim_type);
		gl_v2f();
		gl_v2f();
		gl_v2f();
	gl_end();

	gs_immediate_draw_i
	{
		gs_dyn_array(u8) vertex_data;	
	};

	pipe.raster.layout = {
		FLOAT3,	// Position
		FLOAT2,	// UV
		FLOAT4	// COLOR,
		UINT4	// CUSTOM INFO
	};

	gsi_begin(pipe);

		// Filling out vertex data
		gsi_interleavef(4, 1, 1, 1, 1, stride, offset);
		gsi_interleaveu(4, 10, 100, 20, 40, stride, offset);

		// Vert0
		gsi_3f(0, 0, 0);	 -> internally work against counter for interleaves?
		gsi_2f(0, 0);

		check against interleaves, mod against
		ct += 3 * sizeof(f32)

		// Vert1
		gsi_3f(1, 0, 0);
		gsi_2f(1, 0);
		gsi_4f(1, 1, 1, 1);

		// Vert2
		gsi_3f(1, 1, 0);
		gsi_2f(1, 1);
		gsi_4f(1, 1, 1, 1);

	gsi_end();	// Clears interleaves

	gsdgb_line(gsdbg_i* dbg);	

	layout = [FLOAT3, BYTE4]
	gsdbg_line(x, y, r, g, b, a)
	{
		gsi_begin(line_pipeline); -> This will check against currently bound pipeline and incur draw call if different
		{
			gsi_enable_texture(tex, 0);
			gsi_2f(x, y, 0.f);
			gsi_4b(r, g, b, a);
		}
		gsi_end();
	}

	gsi_enable_texture(tex, 0);

	gsdgb_line();
	gsdgb_boxline();
	for (line in lines) {
		gsdgb_line();
	}

	// Another pipeline to use
	gsdgb_triangle();

	custom pipeline, custom vertex layout

	gsdbg_vattr_type flags[] = {
		GSDBG_VATTR_POSITION,
		GSDBG_VATTR_COLOR,
		GSDBG_VATTR_UV,
		GSDBG_VATTR_FLOAT4
	};

	size_t stride = 3 * f32 + 4 * u8 + 2 * f32 + 4 * f32;
	size_t offset = 3 * f32 + 4 * u8 + 2 * f32;

	[POSITIONS, COLORS, UVS, CUSTOM...]

	gsi_push_pipeline();
	{
		gsdb_triangle();
	}
	gsi_pop_pipeline();

	// Line
	gsdbg_line();
	gsdbg_boxline();
	gsdbg_sphereline();
	gsdbg_circleline();
	gsdbg_rectline();
	gsdbg_triangleline();

	// Text
	gsdgb_text();

	// Fill
	gsdbg_rect();
	gsdbg_box();
	gsdbg_sphere();
	gsdbg_circle();
	gsdbg_triangle();
	gsdbg_cylinder();

	gsi_draw() // Clears everything and submits to command buffer (must be called in between renderpass_begin / renderpass_end calls)

	push_pipeline(custom_pipe, attr_flags*, sz);	-> will incur a draw (if any) (if attr flags are null, then use default flags for pipeline)
	{
		// Will draw triangle using provided attr information
		gsi_triangle();

		// Will interleave this data throughout the vertex declaration
		float ildata[] = {1.f, 0.f, 0.f, 1.f};
		push_vattr_data(attr, ildata, sizeof(ildata));
			gsi_triangle();	
		push_vattr_data();

		// Immediate mode drawing (just buffer commands, allow user to push pipeline)
		// Immediate Shapes (uses immediate mode drawing to draw debug shapes using its own pipelines and commands)

		// Set layout with pipeline
		// Call set of functions to write raw vertex data into buffer 

		// Writing data into buffer stream
		gsi_1f();
		gsi_2f();
		gsi_3f();
		gsi_4f();
		gsi_4u8();
		gsi_3u8();
		gsi_2u8();
		gsi_1u8();
		gsi_4ui();
		gsi_3ui();
		gsi_2ui();
		gsi_1ui();
		gsi_flist(...);
		gsi_u8list(...);
		gsi_ulist(...);
		gsi_ilist(...);

		// What does the default shader look like? 
		v:
			vposition = mvp * a_pos
			vuv 	  = a_uv
			vcolor 	  = a_color
			vnormal   = a_normal
		f:
			fcolor = vcolor * texture(uv)

		// Would be useful to just be able to push a light...
		// Pushing/Popping matrices (do this to pre-multiply your verts with information)(not really necessary though)
	}
	pop_pipeline();	-> will incur draw (if any)

	// Is immediate mode single threaded? Yes, unless you provide your own interface to use.
	// Then you just have to make sure you're not using that interface in multiple places. EZ PZ.

	// Is there a way for this to work with models? Or simple shapes? 
	// Add in new 	

	// Begins render pass
	gs_immediate_begin_pass(gi);
	{
		gs_immediate_push_render_pass(cb, rp, actions, actionsz);
		{	
			// Pipeline has a specific layout, in the vertex data, you can push certain vertex information, depending on layout?
			gs_immediate_push_pipeline(cb, pip);
			{
				gs_immediate_begin(cb);
				{
					// Let's say layout is this: 
						layout = 
							float3, // position
							float2,	// uv
							uint4	// color

					// Then vertex buffer is just a u8 byte buffer that can be uploaded directly to GPU

					// Defines interleaved data format
					gs_immediate_write_3f(cb, x, y, z);
					gs_immediate_write_2f(cb, u, v);
					gs_immediate_write_4ui(cb, r, g, b, a);
				}
				gs_immediate_end(cb);
				gs_immediate_draw(cb);	// Force render
			}
			gs_immediate_pop_pipeline(cb);
		}
		gs_immediate_pop_render_pass(cb);
	}
	gs_immediate_end_pass(cb);

	gs_immediate_draw_triangle();

	Immediate mode rendering utility (for debugging and simple immediate-mode UI)
	Features:
		Shapes: 
			* Line
			* Triangle
			* Rect
			* Circle
			* Cube
			* Sphere
			* Cone
			* Cylinder
		Lighting:
		Matrices:
		Pipeline:
*/ 


#endif // GS_IDRAW_H

