#include "audio/gs_audio.h"

#include "base/gs_engine.h"
#include "platform/gs_platform.h"
#include "common/gs_util.h"

#if defined ( GS_PLATFORM_LINUX )

#include <alsa/asoundlib.h>

typedef struct alsa_sound_output_t
{
	snd_pcm_t* pcm_handle;
	snd_pcm_uframes_t buffer_frames;
	snd_pcm_uframes_t buffer_size;
	snd_pcm_uframes_t period_size;	

    u32 buffer_frame_count;
    u32 channels;
    u32 samples_per_second;
    u32 latency_frame_count;

} alsa_sound_output_t;

void alsa_init( struct gs_audio_i* audio )
{
	gs_audio_data_t* data = audio->data;
	alsa_sound_output_t* output = (alsa_sound_output_t*)data->internal;
	snd_pcm_hw_params_t* hw_params = NULL;

	// We *actually* want 44.1k
	output->samples_per_second = 44100;

	s32 err = 0;	

	// Attempt to open default driver
	if ( (err = snd_pcm_open(&output->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) ) < 0 ) {
		gs_println( "WARNING: Cannot open audio device.");
	}

	if ( (err = snd_pcm_hw_params_malloc( &hw_params ) ) < 0 ) {
		gs_println( "WARNING: Cannot allocate hardware parameter structure.");
	}

	if ( (err = snd_pcm_hw_params_any( output->pcm_handle, hw_params ) ) < 0 ) {
		gs_println( "WARNING: Cannot initialize hardware paramter structure.");
	}

	if ( ( err = snd_pcm_hw_params_set_access(output->pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) ) < 0 ) {
		gs_println( "WARNING: Cannot set hardware paramter access.");
	}

	if ( (err = snd_pcm_hw_params_set_format ( output->pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE )) < 0) {
		gs_println("WARNING: cannot set sample format.");
	}

	if ( (err = snd_pcm_hw_params_set_channels ( output->pcm_handle, hw_params, 2)) < 0) {
		gs_println("WARNING: cannot set channel count.");
	}
	
	if ( (err = snd_pcm_hw_params_set_rate_near ( output->pcm_handle, hw_params, &output->samples_per_second, 0 )) < 0) {
		gs_println ("WARNING: cannot set sample rate.");
	}

	if ( (err = snd_pcm_hw_params ( output->pcm_handle, hw_params)) < 0) {
		gs_println("WARNING: cannot set parameters.");
	}

	// Finally free params memory
	snd_pcm_hw_params_free ( hw_params );

	if ((err = snd_pcm_prepare ( output->pcm_handle )) < 0) {
			gs_println( "WARNING: cannot prepare audio interface for use.");		
	}
}

gs_result audio_init( struct gs_audio_i* audio )
{
	gs_audio_data_t* data = audio->data;

	data->internal = gs_malloc_init( alsa_sound_output_t );
	alsa_sound_output_t* output = (alsa_sound_output_t*)data->internal;

	// Initialize sound output values
	output->channels = 2;
	output->samples_per_second = 48000;
	output->latency_frame_count = 48000;

	// Allocate storage for samples output for hardware
	data->sample_out = gs_malloc( output->samples_per_second * sizeof(s16) * 2 );
	memset(data->sample_out, 0, output->samples_per_second * sizeof(s16) * 2);

	alsa_init( audio );

	return gs_result_success;
}

gs_result audio_shutdown( struct gs_audio_i* audio )
{
	return gs_result_success;
}

gs_result audio_commit( struct gs_audio_i* audio )
{
	// Commit frames to alsa
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;

	// Commit all audio to memory (fill sound buffer)
	u32 samples_to_write 			= __data->sample_count_to_output;
	s16* samples 					= (s16*)__data->sample_out;
	alsa_sound_output_t* output 	= (alsa_sound_output_t*)__data->internal;

	s32 pcm_return = 0;
	while ( ( pcm_return = snd_pcm_writei( output->pcm_handle, samples, samples_to_write ) ) < 0 )
	{
		snd_pcm_prepare( output->pcm_handle );
	}

	return gs_result_success;
}

/*============================================================
// Audio API Construction
============================================================*/

void gs_audio_construct_internal( struct gs_audio_i* audio )
{
	// Audio Init/De-Init Functions
	audio->init 		= &audio_init;
	audio->shutdown 	= &audio_shutdown;
	audio->commit 		= &audio_commit;
}

#endif // GS_PLATFORM_LINUX
