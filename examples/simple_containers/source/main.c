#include <gs.h>

// Forward Decls.
gs_result app_update();		// Use to update your application
gs_result app_init(); 		// Use to init application
gs_result app_shutdown(); 	// Use to shutdown application

// Globals
gs_dyn_array(f32) g_dyn_float_array;

int main( int argc, char** argv )
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size, title, as well as update, init, and shutdown functions to be run. 
	gs_application_desc app = {0};
	app.window_title 		= "Simple Containers";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.update 				= &app_update;
	app.init 				= &app_init;
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

gs_result app_init()
{
	g_dyn_float_array = gs_dyn_array_new( f32 );

	// Add elements to dynamic array
	for ( u32 i = 0; i < 10; ++i )
	{
		gs_dyn_array_push( g_dyn_float_array, (f32)i );
	}

	return gs_result_success;
}

// Update your application here
gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();

	// If we press the escape key, exit the application
	if ( engine->ctx.platform->key_pressed( gs_keycode_esc ) )
	{
		return gs_result_success;
	}

	// Clear array
	if ( engine->ctx.platform->key_pressed( gs_keycode_c ) )
	{
		gs_dyn_array_clear( g_dyn_float_array );
	}

	// Insert incrementing val into array
	if ( engine->ctx.platform->key_pressed( gs_keycode_p ) )
	{
		static f32 v = 0.f;
		gs_dyn_array_push( g_dyn_float_array, v );
		v += 1.f;
	}

	// Print out elements of dynamic array (used a timed action macro so it only prints out at set interval)
	gs_timed_action( 30, 
	{
		gs_printf( "Array: [ " );

		// Loop through all elements of array
		u32 sz = gs_dyn_array_size( g_dyn_float_array );
		for ( u32 i = 0; i < sz; ++i )
		{
			gs_printf( "%.2f", g_dyn_float_array[ i ] );
			gs_printf( i < sz -1 ? ", " : "" );
		}
		gs_printf( " ]\n" );
	});

	// Otherwise, continue
	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	// Release memory for array
	gs_dyn_array_free( g_dyn_float_array );

	return gs_result_success;
}
