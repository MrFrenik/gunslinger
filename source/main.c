#include "gs.h"

#include <GLFW/glfw3.h>

// Global scope
// gs_engine engine = {0};
gs_platform_window_handle window;

gs_result update()
{
	// Draw shit to screen? That'd be nice, wouldn't it?
	gs_timed_action( 100, {
		gs_println( "Updating: FPS: %.2f", gs_engine_instance()->ctx.platform->time.fps );
	});

	// Swap window buffer here, I guess?
	gs_engine* engine = gs_engine_instance();
	engine->ctx.platform->window_swap_buffer( window );

	if ( engine->ctx.platform->mouse_pressed( gs_mouse_lbutton ) ) {
		gs_println( "lmb pressed" );
	}

	if ( engine->ctx.platform->mouse_released( gs_mouse_lbutton ) ) {
		gs_println( "lmb released" );
	}

	if ( engine->ctx.platform->mouse_down( gs_mouse_lbutton ) ) {
		gs_println( "lmb held" );
	}

	return gs_result_in_progress;
}

gs_result shutdown()
{
	return gs_result_success;
}

int main( int argc, char** argv ) 
{
	struct gs_application_context app;
	app.update = &update;
	app.shutdown = &shutdown;

	// Construct internal instance of our engine
	gs_engine engine;
	gs_engine_init( &engine );
	engine.ctx.app = app;

	// This isn't even being called...
	window = engine.ctx.platform->create_window( "game", 800, 600 );

	// Run the engine loop
	gs_result res = engine.run();

	// Run engine
	if ( res != gs_result_success ) 
	{
		gs_println( "Error: Engine did not successfully finish running." );
	}

	gs_println( "Gunslinger exited successfully." );

	return 0;
}


/*
	gs_application_context ctx;

	Application context could hold settings for platform, including video and audiod
*/