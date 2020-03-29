#include <gs.h>

// Globals
_global gs_resource( gs_vertex_buffer ) 	g_vbo = {0};
_global gs_resource( gs_index_buffer ) 		g_ibo = {0};
_global gs_resource( gs_command_buffer ) 	g_cb = {0};
_global gs_resource( gs_shader ) 			g_shader = {0};
_global gs_resource( gs_uniform ) 			u_tex = {0}; 
_global gs_resource( gs_texture ) 			g_tex = {0};

_global const s32 g_texture_width = 800;
_global const s32 g_texture_height = 600;

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
} particle_t;

// For now, all particle information will simply be a value to determine its material id
#define mat_id_empty (u8)0
#define mat_id_sand  (u8)1
#define mat_id_water (u8)2
#define mat_id_stone (u8)3

// Colors
#define mat_col_empty (color_t){ 0, 0, 0, 0}
#define mat_col_sand  (color_t){ 150, 100, 20, 255 }
#define mat_col_water (color_t){ 20, 100, 170, 200 }
#define mat_col_stone (color_t){ 51, 26, 0, 255 }

// World particle data structure
_global particle_t* g_world_particle_data = {0};

// Texture buffers (double buffered color buffers)
_global color_t* g_texture_buffer = {0};

// Frame counter
_global u32 g_frame_counter = 0;

// World physics settings
_global f32 gravity = 3.f;

const char* v_src = "\n"
"#version 330 core\n"
"layout (location = 0) in vec2 a_pos;\n"
"layout (location = 1) in vec2 a_texCoord;\n"
"out vec2 texCoord;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(a_pos, 0.0, 1.0);\n"
"	texCoord = vec2(a_texCoord.x, 1.0 - a_texCoord.y);\n"
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
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction

particle_t particle_empty();
particle_t particle_sand();
particle_t particle_water();
particle_t particle_stone();
void update_particle_sim();
void update_sand( u32 w, u32 h );
void update_water( u32 w, u32 h );
void update_stone( u32 w, u32 h );
void write_data( u32 idx, particle_t );
void render_scene();

s32 random_val( s32 lower, s32 upper )
{
	return ( rand() % (upper - lower + 1) + lower );
}

b32 in_bounds( u32 x, u32 y )
{
	if ( x < 0 || x > ( g_texture_width - 1 ) ||
		 y < 0 || y > ( g_texture_height - 1 ) )
	{
		return false;
	}

	return true;
}

u32 compute_idx( u32 x, u32 y )
{
	return ( y * g_texture_width + x );
}

b32 is_empty( u32 x, u32 y )
{
	return ( in_bounds( x, y ) && g_world_particle_data[ y * g_texture_width + x ].id == mat_id_empty );
}

particle_t get_particle_at( u32 x, u32 y )
{
	return g_world_particle_data[ compute_idx( x, y ) ];
}

int main( int argc, char** argv )
{
	gs_application_desc app = {0};
	app.window_title 		= "SandSim";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;
	app.shutdown 			= &app_shutdown;

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

// Here, we'll initialize all of our application data, which in this case is our graphics resources
gs_result app_init()
{
	// Cache instance of graphics api from engine
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Construct command buffer ( the command buffer is used to allow for immediate drawing at any point in our program )
	g_cb = gfx->construct_command_buffer();

	// Construct shader from our source above
	g_shader = gfx->construct_shader( v_src, f_src );

	// Construct uniform for shader
	u_tex = gfx->construct_uniform( g_shader, "u_tex", gs_uniform_type_sampler2d );

	// Vertex data layout for our mesh (for this shader, it's a single float3 attribute for position)
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

	// Set buffers to 0
	memset( g_texture_buffer, 0, g_texture_width * g_texture_height * sizeof(color_t) );
	memset( g_world_particle_data, 0, g_texture_width * g_texture_height );

	// Construct texture resource from GPU
	gs_texture_parameter_desc t_desc = gs_texture_parameter_desc_default();
	t_desc.mag_filter = gs_nearest;
	t_desc.min_filter = gs_nearest;
	t_desc.generate_mips = false;
	t_desc.width = g_texture_width;
	t_desc.height = g_texture_height;
	t_desc.num_comps = 4;
	t_desc.data = g_texture_buffer;

	g_tex = gfx->construct_texture( t_desc );

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

	// Update particle sim
	update_particle_sim();

	/*===============
	// Render scene
	================*/
	render_scene();

	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}

void update_particle_sim()
{
	// Cache engine subsystem interfaces
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Update frame counter ( loop back to 0 if we roll past u32 max )
	b32 frame_counter_even = ((g_frame_counter % 2) == 0);
	s32 ran = frame_counter_even ? 0 : 1;

	// Rip through read data and update write buffer
	for ( u32 h = g_texture_height - 1; h > 0; --h )
	{
		for ( u32 w = ran ? 0 : g_texture_width - 1; ran ? w < g_texture_width : w > 0; ran ? ++w : --w )
		{
			// Current particle idx
			u32 read_idx = h * g_texture_width + w;

			// Get material of particle at point
			u8 mat_id = g_world_particle_data[ read_idx ].id;

			switch ( mat_id ) {

				case mat_id_sand: update_sand( w, h ); break;
				case mat_id_water: update_water( w, h ); break;
				case mat_id_stone: update_stone( w, h ); break;

				// Do nothing for empty or default case
				default:
				case mat_id_empty: 
				{
				} break;
			}
		}
	}

	// Mouse input for testing
	if ( platform->mouse_down( gs_mouse_lbutton ) )
	{
		gs_vec2 mp = platform->mouse_position();
		f32 mp_x = gs_clamp( mp.x, 0.f, (f32)g_texture_width - 1.f );	
		f32 mp_y = gs_clamp( mp.y, 0.f, (f32)g_texture_height - 1.f );
		u32 max_idx = (g_texture_width * g_texture_height) - 1;
		s32 r_amt = random_val( 1, 100 );
		for ( u32 i = 0; i < r_amt; ++i )
		{
			s32 rx = random_val( -10, 20 );
			s32 ry = random_val( 1, 100 );
			s32 mpx = (s32)gs_clamp( mp_x + (f32)rx, 0.f, (f32)g_texture_width - 1.f );
			s32 mpy = (s32)gs_clamp( mp_y + (f32)ry, 0.f, (f32)g_texture_height - 1.f );
			s32 idx = mpy * (s32)g_texture_width + mpx;
			idx = gs_clamp( idx, 0, max_idx );

			if ( is_empty( mpx, mpy ) )
			{
				write_data( idx, particle_sand() );
			}
		}
	}

	if (  platform->mouse_down( gs_mouse_rbutton ) )
	{
		gs_vec2 mp = platform->mouse_position();
		f32 mp_x = gs_clamp( mp.x, 0.f, (f32)g_texture_width - 1.f );	
		f32 mp_y = gs_clamp( mp.y, 0.f, (f32)g_texture_height - 1.f );
		u32 max_idx = (g_texture_width * g_texture_height) - 1;
		s32 r_amt = random_val( 1, 100 );
		for ( u32 i = 0; i < r_amt; ++i )
		{
			s32 rx = random_val( -10, 20 );
			s32 ry = random_val( 1, 100 );
			s32 mpx = (s32)gs_clamp( mp_x + (f32)rx, 0.f, (f32)g_texture_width - 1.f );
			s32 mpy = (s32)gs_clamp( mp_y + (f32)ry, 0.f, (f32)g_texture_height - 1.f );
			s32 idx = mpy * (s32)g_texture_width + mpx;
			idx = gs_clamp( idx, 0, max_idx );

			if ( is_empty( mpx, mpy ) )
			{
				write_data( idx, particle_water() );
			}
		}
	}

	if ( platform->mouse_down( gs_mouse_mbutton ) )
	{
		gs_vec2 mp = platform->mouse_position();
		f32 mp_x = gs_clamp( mp.x, 0.f, (f32)g_texture_width - 1.f );	
		f32 mp_y = gs_clamp( mp.y, 0.f, (f32)g_texture_height - 1.f );
		u32 max_idx = (g_texture_width * g_texture_height) - 1;
		for ( s32 i = -10 ; i < 10; ++i )
		{
			for ( s32 j = -10 ; j < 10; ++j )
			{
				s32 rx = ((s32)mp_x + j); 
				s32 ry = ((s32)mp_y + i);
				if ( is_empty( rx, ry ) )
				{
					write_data( compute_idx( rx, ry ), particle_stone() );
				}
			}
		}
	}

	if ( platform->key_down( gs_keycode_c ) )
	{
		gs_vec2 mp = platform->mouse_position();
		f32 mp_x = gs_clamp( mp.x, 0.f, (f32)g_texture_width - 1.f );	
		f32 mp_y = gs_clamp( mp.y, 0.f, (f32)g_texture_height - 1.f );
		u32 max_idx = (g_texture_width * g_texture_height) - 1;
		for ( s32 i = -10 ; i < 10; ++i )
		{
			for ( s32 j = -10 ; j < 10; ++j )
			{
				s32 rx = ((s32)mp_x + j); 
				s32 ry = ((s32)mp_y + i);
				if ( in_bounds( rx, ry ) )
				{
					write_data( compute_idx( rx, ry ), particle_empty() );
				}
			}
		}
	}

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

	// Update frame counter
	g_frame_counter = (g_frame_counter + 1) % u32_max;
}

void render_scene()
{
	// Graphics api instance
	gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;

	// Set clear color and clear screen
	f32 clear_color[4] = { 0.2f, 0.2f, 0.2f, 1.f };
	gfx->set_view_clear( g_cb, clear_color );

	// Bind shader
	gfx->bind_shader( g_cb, g_shader );

	// Bind vertex buffer
	gfx->bind_vertex_buffer( g_cb, g_vbo );

	// Bind index buffer
	gfx->bind_index_buffer( g_cb, g_ibo );

	// Bind texture
	gfx->bind_texture( g_cb, u_tex, g_tex, 0 );

	// Draw
	gfx->draw_indexed( g_cb, 6 );

	// Submit command buffer for rendering
	gfx->submit_command_buffer( g_cb );
}

void write_data( u32 idx, particle_t p )
{
	// Write into particle data for id value
	g_world_particle_data[ idx ] = p;

	switch ( p.id )
	{
		case mat_id_sand:  g_texture_buffer[ idx ] = mat_col_sand;    break;
		case mat_id_water: g_texture_buffer[ idx ] = mat_col_water;   break;
		case mat_id_stone: g_texture_buffer[ idx ] = mat_col_stone;   break;
		default:
		case mat_id_empty: {
			g_texture_buffer[ idx ] = mat_col_empty;
		} break;
	}
}

void update_stone( u32 x, u32 y )
{
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;

	// Write data into buffer
	write_data( write_idx, *p );
	if ( write_idx != read_idx ) {
		write_data( read_idx, particle_empty() );
	}
}

void update_sand( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta * 10.f;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;

	p->velocity.y += gravity;

	// Move up to bounds
	while ( !in_bounds( x + p->velocity.x, y + p->velocity.y ) ) {
		p->velocity.y /= 2.f;
		p->velocity.x /= 2.f;
	}

	gs_vec2 norm = gs_vec2_norm( p->velocity );
	f32 len = gs_vec2_len( p->velocity );

	// Want to increment in discrete time steps to move towards velocity. Resolve collision if it occurs
	// Can only increment in steps of 1
	for ( f32 i = 1.f; i <= len; i += 1.f )
	{
		s32 pvx = (s32)((s32)norm.x * (s32)i);
		s32 pvy = (s32)((s32)norm.y * (s32)i);
		s32 new_x = x + pvx;
		s32 new_y = y + pvy;
		b32 p_in_b = in_bounds( new_x, new_y );
		b32 collision = !is_empty( new_x, new_y );

		if ( !collision )
		{
			write_idx = compute_idx( new_x, new_y );
		}

		// Otherwise, we've collided
		else
		{
			// For now, just set velocities to 0
			p->velocity.x = 0.f;
			p->velocity.y = 0.f;

			// Get particle at collision point
			u8 collide_id = get_particle_at( new_x, new_y ).id;	

			if ( collide_id == mat_id_water || collide_id == mat_id_sand || collide_id == mat_id_stone )
			{
				s32 ran = random_val( 0, 1 );
				s32 r = ran ? 1 : -1;
				s32 l = ran ? -1 : 1;
				if ( is_empty( x + r, y + 1 ) ) {
					write_idx = compute_idx( x + r, y + 1 );
					break;
				}
				else if ( is_empty( x + l, y + 1 ) ) {
					write_idx = compute_idx( x + l, y + 1 );
					break;
				}
				else if ( is_empty( x + r, y ) ) {
					write_idx = compute_idx( x + r, y );
					break;
				}
				else if ( is_empty( x + l, y ) ) {
					write_idx = compute_idx( x + l, y );
					break;
				}
			}
		}
	}
	
	// Write data into buffer
	write_data( write_idx, *p );

	if ( write_idx != read_idx ) {
		write_data( read_idx, particle_empty() );
	}
}

void update_water( u32 x, u32 y )
{
	f32 dt = gs_engine_instance()->ctx.platform->time.delta;

	// For water, same as sand, but we'll check immediate left and right as well
	u32 read_idx = compute_idx( x, y );
	particle_t* p = &g_world_particle_data[ read_idx ];
	u32 write_idx = read_idx;

	p->velocity.y += gravity;

	// Move up to bounds
	while ( !in_bounds( x + p->velocity.x, y + p->velocity.y ) ) {
		p->velocity.y /= 2.f;
		p->velocity.x /= 2.f;
	}

	gs_vec2 norm = gs_vec2_norm( p->velocity );
	f32 len = gs_vec2_len( p->velocity );

	// Want to increment in discrete time steps to move towards velocity. Resolve collision if it occurs
	// Can only increment in steps of 1
	for ( f32 i = 1.f; i <= len; i += 1.f )
	{
		s32 pvx = (s32)((s32)norm.x * (s32)i);
		s32 pvy = (s32)((s32)norm.y * (s32)i);
		s32 new_x = x + pvx;
		s32 new_y = y + pvy;
		b32 p_in_b = in_bounds( new_x, new_y );
		b32 collision = !is_empty( new_x, new_y );

		if ( !collision )
		{
			write_idx = compute_idx( new_x, new_y );
		}

		// Otherwise, we've collided
		else
		{
			// Get particle at collision point
			u8 collide_id = get_particle_at( new_x, new_y ).id;	

			if ( collide_id == mat_id_water || collide_id == mat_id_sand || collide_id == mat_id_stone )
			{
				p->velocity.y = 0.f;
				p->velocity.x = 0.f;

				// Figure out collision resolution vector
				// Resolve collision
				s32 ran = random_val( 0, 1 );
				s32 r = ran ? 1 : -1;
				s32 l = ran ? -1 : 1;
				if ( is_empty( x + r, y + 1 ) ) {
					write_idx = compute_idx( x + r, y + 1 );
					break;
				}
				else if ( is_empty( x + l, y + 1 ) ) {
					write_idx = compute_idx( x + l, y + 1 );
					break;
				}
				else if ( is_empty( x + r, y ) ) {
					write_idx = compute_idx( x + r, y );
					break;
				}
				else if ( is_empty( x + l, y ) ) {
					write_idx = compute_idx( x + l, y );
					break;
				}
			}
		}
	}
	
	// Write data into buffer
	write_data( write_idx, *p );
	if ( write_idx != read_idx ) {
		write_data( read_idx, particle_empty() );
	}
}

particle_t particle_empty()
{
	particle_t p = {0};
	p.id = mat_id_empty;
	return p;
}

particle_t particle_sand()
{
	particle_t p = {0};
	p.id = mat_id_sand;
	return p;
}

particle_t particle_water()
{
	particle_t p = {0};
	p.id = mat_id_water;
	return p;
}

particle_t particle_stone()
{
	particle_t p = {0};
	p.id = mat_id_stone;
	return p;
}


/*
	// For each particle, check against collisions from other particles

	// For each cell, do checks
	
	 // Might as well do this the right way from the get go...

*/


void update_sim()
{
	// Check particle vs. particle collisions	
	// I think this is where double buffering MIGHT actually come in handy...

	// Naive way first

	// Loop through all particles, do collisions with all other particles. What could go wrong?
	// for ( u32 h = 0; h < g_texture_height; ++h )
	// {
	// 	for ( u32  w = 0; w < g_texture_width; ++w; )
	// 	{

	// 	}	
	// }

}

















