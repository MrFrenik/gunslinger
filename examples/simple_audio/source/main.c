#include <gs.h>

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application

// Don't like that these are resources while all the graphics objects are not. Kinda ruins it for me.
_global gs_handle_audio_source 		g_src = {0};
_global gs_handle_audio_instance 	g_inst = {0};

// Instances need to be operated on in a separate thread.
// Can get copy of instance data.
// Can get copy of raw source data as well.
// That'll have to do.

/*
	Make an audio instance buffer? Would have to copy instance data into it, I believe, each frame.

	// Pass up an audio instance buffer to be used? This gets weird. Would need to use handles internally or something like that to manage the buffer.
	// This gets out of hand pretty quickly.
	// Don't want to copy raw source data either, obviously.
*/

int main( int argc, char** argv )
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size, title, as well as update, init, and shutdown functions to be run. 
	gs_application_desc app = {0};
	app.window_title 		= "Simple Audio";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;
	app.update 				= &app_update;

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
	// Cache apis
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;

	// Constuct audio resource to play
	g_src = audio->load_audio_source_from_file( platform->file_exists( "./assets/cold_morning_tx.mp3" ) ? 
		"./assets/cold_morning_tx.mp3" : "./../assets/cold_morning_tx.mp3" );

	// Construct instance source and play on loop. Forever.
	gs_audio_instance_data_t inst = gs_audio_instance_data_new( g_src );
	inst.volume = 0.8f;						// Range from [0.f, 1.f]
	inst.loop = true;						// Tell whether or not audio should loop 
	inst.persistent = true;					// Whether or not instance should stick in memory after completing 
	g_inst = audio->play_instance_data( inst );

	return gs_result_success;
}

gs_result app_update()
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;	

	if ( platform->key_pressed( gs_keycode_esc ) ) 
	{
		return gs_result_success;
	}

	return gs_result_in_progress;
}
