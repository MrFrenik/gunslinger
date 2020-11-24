#include <gs.h>

/*
	Simple c++ example
*/

// Forward Decls.
gs_result app_update();		// Use to update your application

int main(int argc, char** argv)
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size, title, as well as update, init, and shutdown functions to be run. 
	gs_application_desc_t app = {0};
	app.window_title 		= "Hello, CPP";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.update 				= &app_update;

	// Construct internal instance of our engine
	gs_engine_t* engine = gs_engine_construct(app);

	// Run the internal engine loop until completion
	gs_result res = engine->run();

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
	// Grab global instance of engine
	gs_engine_t* engine = gs_engine_instance();

	// If we press the escape key, exit the application
	if (engine->ctx.platform->key_pressed(gs_keycode_esc))
	{
		return gs_result_success;
	}

	// Otherwise, continue
	return gs_result_in_progress;
}
