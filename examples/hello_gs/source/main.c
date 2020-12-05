
/*================================================================
	* Copyright: 2020 John Jackson
	* HellGS

	A Bare bones application for getting started using `gunslinger`.
	Creates an appplication context, an engine context, and then 
	opens a main window for you using the rendering context.

	Press `esc` to exit the application.
=================================================================*/

#include <gs.h>

// Forward Decls.
gs_result app_update();		// Use to update your application

int main(int argc, char** argv)
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size, title, as well as update, init, and shutdown functions to be run.
	gs_application_desc_t app = {0};
	app.window_title 		= "Hello, Gunslinger";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.window_flags 		= gs_window_flags_resizable;
	app.update 				= &app_update;

	// Construct internal instance of engine, register app, then run engine loop until completion
	gs_result res = gs_engine_construct(app)->run();

	// Check result of engine after exiting loop
	if (res != gs_result_success) 
	{
		gs_println("Error: Engine did not successfully finish running.");
		return -1;
	}

	gs_println("Gunslinger exited successfully.");

	return 0;	
}

// Update your application here
gs_result app_update()
{
	// Platform layer api
	gs_platform_i* platform = gs_engine_subsystem(platform);

	// If we press the escape key, exit the application
	if (platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	// Otherwise, continue
	return gs_result_in_progress;
}
