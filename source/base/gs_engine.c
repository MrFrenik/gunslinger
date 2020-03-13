#include "base/gs_engine.h"
#include "common/gs_util.h"
#include "platform/gs_platform.h"
#include "math/gs_math.h"

// Global instance of gunslinger engine ( ...THERE AN ONLY BE ONE )
// Don't like this...
gs_engine* gs_engine_instance_g = { 0 };

// Function forward declarations
gs_result gs_engine_run();
gs_result gs_engine_shutdown();
gs_result gs_default_app_update();
gs_result gs_default_app_shutdown();
void __gs_default_init_platform();

void gs_engine_init( gs_engine* engine )
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
		gs_assert( gs_engine_instance_g->ctx.platform );

		// Default initialization for platform here
		__gs_default_init_platform( gs_engine_instance_g->ctx.platform );

		// Initialize plaform layer
		gs_engine_instance_g->ctx.platform->init( gs_engine_instance_g->ctx.platform );

		// Default application context
		gs_engine_instance_g->ctx.app.update = &gs_default_app_update;
		gs_engine_instance_g->ctx.app.shutdown = &gs_default_app_shutdown;

		// Might not want to have these by default...
		// gs_engine_instance_g->ctx.input = gs_engine_instance_g->ctx.platform->create_input();

		// gs_engine_instance_g->ctx.assets = gs_construct_heap( gs_asset_subsystem );
		// gs_asset_subsystem_init( gs_engine_instance_g->ctx.assets, "./assets/" );

		// Construct graphics
		// gs_engine_instance_g->ctx.graphics = gs_graphics_subsystem_construct();

		// gs_assert( gs_engine_instance_g->ctx.input );

		// Just assert these for now
		gs_assert( gs_engine_instance_g->ctx.platform->process_input );
		gs_assert( gs_engine_instance_g->ctx.platform->key_pressed );
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

		f64 start_time = gs_engine_instance()->ctx.platform->get_time();

		gs_platform_i* platform = gs_engine_instance()->ctx.platform;

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

		// Update graphics system
		// gs_graphics_subsystem_update( gs_engine_instance_g->ctx.graphics );

		f64 end_time = gs_engine_instance()->ctx.platform->get_time();
		// gs_println( "st: %.2f, et: %.2f", start_time, end_time );

		// Frame rate locking
		curr_ticks = gs_engine_instance()->ctx.platform->ticks();
		u32 ticks = curr_ticks - prev_ticks;
		// gs_println( "ct: %zu, prev_ticks: %zu", curr_ticks, prev_ticks );
		f32 dt = (f32)( ticks ) / 1000.f;
		prev_ticks = curr_ticks; 

		gs_engine_instance()->ctx.platform->time.delta_time = dt;
		gs_engine_instance()->ctx.platform->time.total_elapsed_time += dt;
		gs_engine_instance()->ctx.platform->time.fps = 1000.f / (f32)ticks;

		// Clamp frame rate to ease up on CPU usage
		// Not sure how to handle this correctly just yet...
		const f32 locked_frame_rate = gs_engine_instance()->ctx.platform->time.max_fps;
		if ( (f32)ticks < 1000.0f / locked_frame_rate )
		{
			gs_engine_instance()->ctx.platform->sleep( (u32)( 1000.f / locked_frame_rate - (f32)ticks ) );
		}

		// Swap window buffer
		// gs_engine_instance()->ctx.platform->window_swap_buffer( gs_engine_instance()->ctx.platform->window );
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

void __gs_default_init_platform( struct gs_platform_i* platform )
{
	// Initialize slot array for window management
	gs_engine_instance_g->ctx.platform->windows = gs_slot_array_new( gs_platform_window_ptr );

	/*============================
	// Platform Input
	============================*/
	platform->update_input 		= &__gs_platform_update_input;
	platform->press_key 		= &__gs_platform_press_key;
	platform->release_key   	= &__gs_platform_release_key;
	platform->was_key_down 		= &__gs_platform_was_key_down;
	platform->key_pressed 		= &__gs_platform_key_pressed;
	platform->key_down 			= &__gs_platform_key_down;
	platform->key_released 	 	= &__gs_platform_key_released;

	platform->press_mouse_button 	= &__gs_platform_press_mouse_button;
	platform->release_mouse_button 	= &__gs_platform_release_mouse_button;
	platform->was_mouse_down 		= &__gs_platform_was_mouse_down;
	platform->mouse_pressed 		= &__gs_platform_mouse_pressed;
	platform->mouse_down 			= &__gs_platform_mouse_down;
	platform->mouse_released 		= &__gs_platform_mouse_released;

	platform->mouse_delta 			= &__gs_platform_mouse_delta;
	platform->mouse_position 		= &__gs_platform_mouse_position;
	platform->mouse_position_x_y 	= &__gs_platform_mouse_position_x_y;
	platform->mouse_wheel 			= &__gs_platform_mouse_wheel;

	/*============================
	// Platform UUID
	============================*/
	platform->generate_uuid 	= &__gs_platform_generate_uuid;
	platform->uuid_to_string 	= &__gs_platform_uuid_to_string;
	platform->hash_uuid 		= &__gs_platform_hash_uuid;

	/*============================
	// Platform File IO
	============================*/

	platform->read_file_contents_into_string_null_term 	= &__gs_platform_read_file_contents_into_string_null_term;
	platform->write_str_to_file 						= &__gs_platform_write_str_to_file;

	// Default world time initialization
	gs_engine_instance_g->ctx.platform->time.max_fps = 60.f;
	gs_engine_instance_g->ctx.platform->time.total_elapsed_time = 0.f;
	gs_engine_instance_g->ctx.platform->time.delta_time = 0.f;
	gs_engine_instance_g->ctx.platform->time.fps = 0.f;
}
