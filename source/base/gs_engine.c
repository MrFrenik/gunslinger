#include "base/gs_engine.h"
#include "common/gs_util.h"
#include "platform/gs_platform.h"
#include "graphics/gs_graphics.h"
#include "math/gs_math.h"

// Global instance of gunslinger engine ( ...THERE AN ONLY BE ONE )
_global gs_engine* gs_engine_instance_g = { 0 };

// Function forward declarations
gs_result gs_engine_run();
gs_result gs_engine_shutdown();
gs_result gs_default_app_update();
gs_result gs_default_app_shutdown();

gs_resource_handle window;

void gs_engine_init( gs_engine* engine, gs_application_desc app_desc )
{
	// Set instance
	if ( gs_engine_instance_g == NULL )
	{
		// NOTE(john): malloc DOES NOT initialize any data, so cannot rely on the memory actually being null,
		// So must explicitly zero out the memory
		// gs_engine_instance_g = gs_malloc_init( gs_engine );

		// Set instance
		gs_engine_instance_g = engine;

		// Initialize the meta class registry
		// Want a way to add user meta class information as well as the engine's (haven't considered how that looks yet)
		// gs_meta_class_registry_init( &gs_engine_instance_g->ctx.registry );

		// gs_assert( gs_engine_instance_g->ctx.registry.classes != NULL );	

		// Set up function pointers
		gs_engine_instance_g->run 		= &gs_engine_run;
		gs_engine_instance_g->shutdown 	= &gs_engine_shutdown;

		// Need to have video settings passed down from user
		gs_engine_instance_g->ctx.platform = gs_platform_construct();

		// Default initialization for platform here
		__gs_default_init_platform( gs_engine_instance_g->ctx.platform );

		// Construct window
		gs_engine_instance()->ctx.platform->create_window( "Test", 800, 600 );
		// window = gs_engine_instance()->ctx.platform->create_window( "Test", 800, 600 );

		// Construct graphics api 
		gs_engine_instance_g->ctx.graphics = gs_graphics_construct();

		// Initialize graphics here
		gs_engine_instance_g->ctx.graphics->init( gs_engine_instance_g->ctx.graphics );

		// Default application context
		gs_engine_instance_g->ctx.app.update = app_desc.update == NULL ? &gs_default_app_update : app_desc.update;
		gs_engine_instance_g->ctx.app.shutdown = app_desc.shutdown == NULL ? &gs_default_app_shutdown : app_desc.shutdown;

		if ( app_desc.init )
		{
			app_desc.init();
		}

		// gs_engine_instance_g->ctx.assets = gs_construct_heap( gs_asset_subsystem );
		// gs_asset_subsystem_init( gs_engine_instance_g->ctx.assets, "./assets/" );

		// Construct graphics
		// gs_engine_instance_g->ctx.graphics = gs_graphics_subsystem_construct();
	}
	// return gs_engine_instance_g;
}

gs_result gs_engine_run()
{
	// Main engine loop
	while ( true )
	{
		static u32 curr_ticks = 0; 
		static u32 prev_ticks = 0;

		// Cache platform pointer
		gs_platform_i* platform = gs_engine_instance()->ctx.platform;

		// Cache times at start of frame
		platform->time.current 	= platform->elapsed_time();  
		platform->time.update 	= platform->time.current - platform->time.previous;
		platform->time.previous = platform->time.current;

		// Update platform input from previous frame		
		platform->update_input( &platform->input );

		// Process input for this frame
		if ( platform->process_input() != gs_result_in_progress )
		{
			return ( gs_engine_instance()->shutdown() );
		} 

		// For now, just shutdown when escape is pressed
		if ( platform->key_pressed( gs_keycode_esc ) )
		{
			return ( gs_engine_instance()->shutdown() );
		}

		// Process application context
		if ( gs_engine_instance()->ctx.app.update() != gs_result_in_progress )
		{
			// Shutdown engine and return
			return ( gs_engine_instance()->shutdown() );
		}

		if ( gs_engine_instance()->ctx.graphics && gs_engine_instance()->ctx.graphics->update )
		{
			gs_engine_instance()->ctx.graphics->update();
		}

		// NOTE(John): This won't work forever. Must change eventually.
		// Swap all platform window buffers? Sure...
		gs_for_range_i( gs_dyn_array_size( platform->active_window_handles ) )
		{
			platform->window_swap_buffer( platform->active_window_handles[ i ] );
		}

		// platform->window_swap_buffer( window );

		// Frame locking
	    platform->time.current 	= platform->elapsed_time();
	    platform->time.render 	= platform->time.current - platform->time.previous;
	    platform->time.previous = platform->time.current;
	    platform->time.frame 	= platform->time.update + platform->time.render; 			// Total frame time

	    f32 target = ( 1000.f / platform->time.max_fps );

	    if ( platform->time.frame < target )
	    {
	    	platform->sleep( (f32)( target - platform->time.frame ) );

	    	platform->time.current = platform->elapsed_time();
	    	f64 wait_time = platform->time.current - platform->time.previous;
	    	platform->time.previous = platform->time.current;
	    	platform->time.frame += wait_time;
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











