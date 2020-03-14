#include "gs.h"

#include <GLFW/glfw3.h>

// Global scope
// gs_engine engine = {0};
gs_platform_window_handle window;

gs_result update()
{
	gs_engine* engine = gs_engine_instance();
	gs_platform_i* platform = engine->ctx.platform;

	// Draw shit to screen? That'd be nice, wouldn't it?
	gs_timed_action( 0, {
		gs_println( "fps: %.2f, mp: <%.2f, %.2f>", 
				platform->time.fps, 
				platform->input.mouse.position.x, 
				platform->input.mouse.position.y 
		);
	});

	if ( engine->ctx.platform->key_pressed( gs_keycode_w ) ) {
		platform->set_window_size( window, 200, 500 );
	}
	if ( engine->ctx.platform->key_pressed( gs_keycode_s ) ) {
		platform->set_window_size( window, 100, 200 );
	}
	if ( engine->ctx.platform->key_pressed( gs_keycode_a ) ) {
		platform->set_window_size( window, 500, 200 );
	}
	if ( engine->ctx.platform->key_pressed( gs_keycode_d ) ) {
		platform->set_window_size( window, 400, 300 );
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

	// Construct window
	window = engine.ctx.platform->create_window( "Test", 800, 600 );

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