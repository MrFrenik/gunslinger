#include <gs.h>

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application

_global gs_resource( gs_audio_source ) g_music_src = {0};
_global gs_resource( gs_audio_instance ) g_music = {0};

int main( int argc, char** argv )
{
	// This is our app description. It gives internal hints to our engine for various things like 
	// window size, title, as well as update, init, and shutdown functions to be run. 
	gs_application_desc app = {0};
	app.window_title 		= "Simple Audio";
	app.window_width 		= 800;
	app.window_height 		= 600;
	app.init 				= &app_init;

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
	g_music_src = audio->load_audio_source_from_file( platform->file_exists( "./assets/cold_morning_tx.mp3" ) ? 
		"./assets/cold_morning_tx.mp3" : "./../assets/cold_morning_tx.mp3" );

	// Construct instance source and play music.
	g_music = audio->play( g_music_src );

	return gs_result_success;
}
