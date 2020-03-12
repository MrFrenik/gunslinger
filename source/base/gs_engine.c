#include "base/gs_engine.h"
#include "common/gs_util.h"
#include "platform/gs_platform.h"
#include "math/gs_math.h"

// Global instance of gunslinger engine ( ...THERE AN ONLY BE ONE )
gs_engine* gs_engine_instance_g = { 0 };

// Function forward declarations
gs_result gs_engine_run();
gs_result gs_engine_shutdown();
gs_result gs_default_app_update();
gs_result gs_default_app_shutdown();

gs_engine* gs_engine_construct()
{
	// Set instance
	if ( gs_engine_instance_g == NULL )
	{
		// NOTE(john): malloc DOES NOT initialize any data, so cannot rely on the memory actually being null,
		// So must explicitly zero out the memory
		gs_engine_instance_g = gs_malloc_init( gs_engine );

		// Initialize the meta class registry
		// Want a way to add user meta class information as well as the engine's (haven't considered how that looks yet)
		// gs_meta_class_registry_init( &gs_engine_instance_g->ctx.registry );

		// gs_assert( gs_engine_instance_g->ctx.registry.classes != NULL );	

		// Set up function pointers
		gs_engine_instance_g->run 		= &gs_engine_run;
		gs_engine_instance_g->shutdown 	= &gs_engine_shutdown;

		// Need to have video settings passed down from user
		gs_engine_instance_g->ctx.platform = gs_platform_construct();
		gs_assert( gs_engine_instance_g->ctx.platform );

		// Initialize plaform layer
		gs_engine_instance_g->ctx.platform->init( gs_engine_instance_g->ctx.platform );

		// Default application context
		gs_engine_instance_g->ctx.app.update = &gs_default_app_update;
		gs_engine_instance_g->ctx.app.shutdown = &gs_default_app_shutdown;

		// Might not want to have these by default...
		gs_engine_instance_g->ctx.input = gs_engine_instance_g->ctx.platform->create_input();

		// Default world time initialization
		gs_engine_instance_g->ctx.time.max_fps = 60.f;
		gs_engine_instance_g->ctx.time.total_elapsed_time = 0.f;
		gs_engine_instance_g->ctx.time.delta_time = 0.f;
		gs_engine_instance_g->ctx.time.fps = 0.f;

		// gs_engine_instance_g->ctx.assets = gs_construct_heap( gs_asset_subsystem );
		// gs_asset_subsystem_init( gs_engine_instance_g->ctx.assets, "./assets/" );

		// Construct graphics
		// gs_engine_instance_g->ctx.graphics = gs_graphics_subsystem_construct();

		gs_assert( gs_engine_instance_g->ctx.input );
	}
	
	return gs_engine_instance_g;
}

gs_result gs_engine_run()
{
	// Main engine loop
	while ( true )
	{
		static u32 curr_ticks = 0; 
		static u32 prev_ticks = 0;

		curr_ticks = gs_engine_instance()->ctx.platform->ticks();
		u32 ticks = curr_ticks - prev_ticks;
		f32 dt = (f32)( ticks ) / 1000.f;
		prev_ticks = curr_ticks; 

		gs_engine_instance()->ctx.time.delta_time = dt;
		gs_engine_instance()->ctx.time.total_elapsed_time += dt;
		gs_engine_instance()->ctx.time.fps = 1000.f / (f32)ticks;

		// Process all input
		if ( gs_engine_instance()->ctx.platform->process_input( gs_engine_instance()->ctx.input ) != gs_result_in_progress )
		{
			return ( gs_engine_instance()->shutdown() );
		} 

		// For now, just shutdown when escape is pressed
		if ( gs_engine_instance()->ctx.platform->key_pressed( gs_engine_instance()->ctx.input, gs_keycode_esc ) )
		{
			return ( gs_engine_instance()->shutdown() );
		}

		// Process application context
		if ( gs_engine_instance()->ctx.app.update() != gs_result_in_progress )
		{
			// Shutdown engine and return
			return ( gs_engine_instance()->shutdown() );
		}

		// Update graphics system
		// gs_graphics_subsystem_update( gs_engine_instance_g->ctx.graphics );

		// Clamp frame rate to ease up on CPU usage
		const f32 locked_frame_rate = gs_engine_instance()->ctx.time.max_fps;
		if ( (f32)ticks < 1000.0f / locked_frame_rate )
		{
			gs_engine_instance()->ctx.platform->delay( (u32)( 1000.f / locked_frame_rate - (f32)ticks ) );
		}
	}

	// Shouldn't hit here
	gs_assert( false );
	return gs_result_failure;
}

gs_result gs_engine_shutdown()
{
	// Shutdown application
	return ( gs_engine_instance()->ctx.app.shutdown() );
}

gs_engine* gs_engine_instance()
{
	return gs_engine_instance_g;
}

gs_result gs_default_app_update()
{
	return gs_result_in_progress;
}

gs_result gs_default_app_shutdown()
{
	return gs_result_success;
}
