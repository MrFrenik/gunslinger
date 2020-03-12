#include "gs.h"

gs_result update()
{
	// Draw shit to screen? That'd be nice, wouldn't it?
	gs_timed_action( 10, {
		gs_println( "Updating..." );
	});

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
	gs_engine* engine = gs_engine_construct( app );
	engine->ctx.app = app;

	// Create window ( this should give an opaque handle, not a pointer )
	// gs_handle window = engine->ctx.platform->create_window( "game", 800, 600 );
	struct gs_platform_window* window = engine->ctx.platform->create_window( "game", 800, 600 );

	// Destroy this window
	// engine->ctx.platform->destroy_window( window );

	// Run the engine loop
	gs_result res = engine->run();

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