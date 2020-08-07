#include <gs.h>

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application
gs_result app_shutdown();	// Use to shutdown your appliaction

_global b32 g_app_running = true;
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
	app.update 				= &app_update;
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

void app_close_window_callback( void* window )
{
	g_app_running = false;
}

gs_result app_init()
{
	// Cache apis
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;

	// Set callback for when window close button is pressed
	platform->set_window_close_callback( platform->main_window(), &app_close_window_callback );

	// Constuct audio resource to play
	g_music_src = audio->load_audio_source_from_file( platform->file_exists( "./assets/cold_morning_tx.mp3" ) ? 
		"./assets/cold_morning_tx.mp3" : "./../assets/cold_morning_tx.mp3" );

	// Construct instance source and play on loop. Forever.
	gs_audio_instance_data_t inst = gs_audio_instance_data_new( g_music_src );
	inst.loop = true;
	inst.persistent = true;
	g_music = audio->play( inst );

	return gs_result_success;
}

gs_result app_update()
{
	// Grab global instance of engine
	gs_engine* engine = gs_engine_instance();
	gs_platform_i* platform = engine->ctx.platform;
	gs_audio_i* audio = engine->ctx.audio;

	// If we press the escape key, exit the application
	if ( platform->key_pressed( gs_keycode_esc ) || !g_app_running )
	{
		return gs_result_success;
	}

	// Pause audio
	if ( platform->key_pressed( gs_keycode_p ) ) 
	{
		audio->pause( g_music );
	}

	// Resume audio
	if ( platform->key_pressed( gs_keycode_r ) ) 
	{
		audio->resume( g_music );
	}

	// Restart audio
	if ( platform->key_pressed( gs_keycode_b ) ) 
	{
		audio->restart( g_music );
	}

	// Stop audio
	if ( platform->key_pressed( gs_keycode_s ) ) 
	{
		audio->stop( g_music );
	}

	// Volume up
	if ( platform->key_pressed( gs_keycode_up ) )
	{
		f32 vol = audio->get_volume( g_music );
		audio->set_volume( g_music, gs_min( vol + 0.1f, 1.f ) );
	}

	// Volume down
	if ( platform->key_pressed( gs_keycode_down ) )
	{
		f32 vol = audio->get_volume( g_music );
		audio->set_volume( g_music, gs_max( vol - 0.1f, 0.f ) );
	}

	// Otherwise, continue
	return gs_result_in_progress;
}

gs_result app_shutdown()
{
	gs_println( "Goodbye, Gunslinger." );
	return gs_result_success;
}
