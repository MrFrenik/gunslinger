#include "base/gs_engine.h"
#include "common/gs_util.h"
#include "platform/gs_platform.h"
#include "graphics/gs_graphics.h"
#include "audio/gs_audio.h"
#include "math/gs_math.h"

// Global instance of gunslinger engine ( ...THERE CAN ONLY BE ONE )
_global gs_engine* gs_engine_instance_g = { 0 };

// Function forward declarations
gs_result gs_engine_run();
gs_result gs_engine_shutdown();
gs_result gs_default_app_update();
gs_result gs_default_app_shutdown();

gs_resource_handle window;

gs_engine* gs_engine_construct( gs_application_desc app_desc )
{
	if ( gs_engine_instance_g == NULL )
	{
		// Construct instance
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

		// Default initialization for platform here
		__gs_default_init_platform( gs_engine_instance_g->ctx.platform );

		// Set frame rate for application
		if ( app_desc.frame_rate > 0.f ) {
			gs_engine_instance_g->ctx.platform->time.max_fps = app_desc.frame_rate;
		}

		// Set vsync for video
		gs_engine_instance_g->ctx.platform->enable_vsync( app_desc.enable_vsync );

		// Construct window
		gs_engine_instance()->ctx.platform->create_window( app_desc.window_title, app_desc.window_width, app_desc.window_height );

		// Construct graphics api 
		gs_engine_instance_g->ctx.graphics = gs_graphics_construct();

		// Initialize graphics here
		gs_engine_instance_g->ctx.graphics->init( gs_engine_instance_g->ctx.graphics );

		// Construct audio api
		gs_engine_instance_g->ctx.audio = gs_audio_construct();

		// Initialize audio
		gs_engine_instance_g->ctx.audio->init( gs_engine_instance_g->ctx.audio );

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

	return gs_engine_instance_g;
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
		gs_graphics_i* gfx = gs_engine_instance()->ctx.graphics;
		gs_audio_i* audio = gs_engine_instance()->ctx.audio;

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

		// Process application context
		if ( gs_engine_instance()->ctx.app.update() != gs_result_in_progress )
		{
			// Shutdown engine and return
			return ( gs_engine_instance()->shutdown() );
		}

		// Audio update and commit
		if ( audio )
		{
			if ( audio->update) {
				audio->update( audio );
			}
			if ( audio->commit ) {
				audio->commit( audio );
			}
		}

		// Graphics update and commit
		if ( gfx && gfx->update )
		{
			gfx->update( gfx );
		}

		// NOTE(John): This won't work forever. Must change eventually.
		// Swap all platform window buffers? Sure...
		gs_for_range_i( gs_dyn_array_size( platform->active_window_handles ) )
		{
			platform->window_swap_buffer( platform->active_window_handles[ i ] );
		}

		// Frame locking
	    platform->time.current 	= platform->elapsed_time();
	    platform->time.render 	= platform->time.current - platform->time.previous;
	    platform->time.previous = platform->time.current;
	    platform->time.frame 	= platform->time.update + platform->time.render; 			// Total frame time
	    platform->time.delta 	= platform->time.frame / 1000.f;

	    f32 target = ( 1000.f / platform->time.max_fps );

	    if ( platform->time.frame < target )
	    {
	    	platform->sleep( (f32)( target - platform->time.frame ) );
	    	
	    	platform->time.current = platform->elapsed_time();
	    	f64 wait_time = platform->time.current - platform->time.previous;
	    	platform->time.previous = platform->time.current;
	    	platform->time.frame += wait_time;
		    platform->time.delta 	= platform->time.frame / 1000.f;
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











