#include <gs.h>

// Forward Decls.
gs_result app_init();		// Use to init your application
gs_result app_update();		// Use to update your application

// Resource handles for internal audio data. Since audio must run on a separate thread, this is necessary.
_global gs_audio_source_t* 			g_src = {0};
_global gs_handle_audio_instance 	g_inst = {0};

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
	// Fill out instance data to pass into audio subsystem
	gs_audio_instance_data_t inst = gs_audio_instance_data_new( g_src );
	inst.volume = 0.8f;						// Range from [0.f, 1.f]
	inst.loop = true;						// Tell whether or not audio should loop 
	inst.persistent = true;					// Whether or not instance should stick in memory after completing, if not then will be cleared from memory
	g_inst = audio->construct_instance( inst );
	audio->play( g_inst );

	return gs_result_success;
}

gs_result app_update()
{
	// Cache necessary APIs
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;	
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;

	if ( platform->key_pressed( gs_keycode_esc ) ) 
	{
		return gs_result_success;
	}

	if ( platform->key_pressed( gs_keycode_p ) )
	{
		if ( audio->is_playing( g_inst ) ) 
		{
			audio->pause( g_inst );
		} 
		else 
		{
			audio->play( g_inst );
		}
	}

	// Restart instance
	if ( platform->key_pressed( gs_keycode_r ) )
	{
		audio->restart( g_inst );
	}

	// Stop audio (sets position back to beginning)
	if ( platform->key_pressed( gs_keycode_s ) )
	{
		audio->stop( g_inst );
	}

	// Volume up
	if ( platform->key_pressed( gs_keycode_up ) )
	{
		f32 cur_vol = audio->get_volume( g_inst );
		audio->set_volume( g_inst, cur_vol + 0.1f );
	}

	// Volume down
	if ( platform->key_pressed( gs_keycode_down ) )
	{
		f32 cur_vol = audio->get_volume( g_inst );
		audio->set_volume( g_inst, cur_vol - 0.1f );
	}

	gs_audio_instance_data_t id = audio->get_instance_data( g_inst );
	s32 sample_count = g_src->sample_count;
	s32 sample_rate = g_src->sample_rate;
	s32 num_channels = g_src->channels;

	gs_timed_action( 10, 
	{
		s32 min, sec;
		char buf[256] = gs_default_val();

		// Runtime of source
		audio->get_runtime( g_src, &min, &sec );
		gs_snprintf( buf, 256, sec < 10 ? "%d:0%d" : "%d:%d", min, sec );
		gs_println( "Runtime: %s", buf );

		// Get current play position
		audio->convert_to_runtime( sample_count, sample_rate, 
			num_channels, id.sample_position, &min, &sec );

		gs_snprintf( buf, 256, sec < 10 ? "%d:0%d" : "%d:%d", min, sec );
		gs_println( "Play Time: %s", buf );
	});

	return gs_result_in_progress;
}







