#include "audio/gs_audio.h"
#include "base/gs_engine.h"
#include "platform/gs_platform.h"
#include "common/gs_util.h"

#include <windows.h>
#include <windows.h>
#include <windowsx.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>

#include "stb/stb_vorbis.c"

static const GUID IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4c32, 0xB1, 0x78, 0xC2, 0xF5, 0x68, 0xA7, 0x03, 0xB2};
static const GUID IID_IAudioRenderClient = {0xF294ACFC, 0x3146, 0x4483, 0xA7, 0xBF, 0xAD, 0xDC, 0xA7, 0xC2, 0x60, 0xE2};
static const GUID CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, 0x8E, 0x3D, 0xC4, 0x57, 0x92, 0x91, 0x69, 0x2E};
static const GUID IID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, 0xA7, 0x46, 0xDE, 0x8D, 0xB6, 0x36, 0x17, 0xE6};
static const GUID PcmSubformatGuid = {STATIC_KSDATAFORMAT_SUBTYPE_PCM};

#ifndef AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
#endif

#ifndef AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY
#define AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY 0x08000000
#endif

#define SOUND_LATENCY_FPS 15
#define REFTIMES_PER_SEC 10000000

#define CO_CREATE_INSTANCE(name) HRESULT name(REFCLSID rclsid, LPUNKNOWN *pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
typedef CO_CREATE_INSTANCE(CoCreateInstance_);
CO_CREATE_INSTANCE(CoCreateInstanceStub)
{
    return 1;
}
_global CoCreateInstance_ *CoCreateInstanceProc = CoCreateInstanceStub;

#define CO_INITIALIZE_EX(name) HRESULT name(LPVOID pvReserved, DWORD dwCoInit)
typedef CO_INITIALIZE_EX(CoInitializeEx_);
CO_INITIALIZE_EX(CoInitializeExStub)
{
    return 1;
}
_global CoInitializeEx_ *CoInitializeExProc = CoInitializeExStub;

typedef struct wasapi_sound_output_t
{
    b32 initialized;
	 IMMDeviceEnumerator *device_enum;
    IMMDevice *device;
    IAudioClient *audio_client;
    IAudioRenderClient *audio_render_client;
    REFERENCE_TIME sound_buffer_duration;
    u32 buffer_frame_count;
    u32 channels;
    u32 samples_per_second;
    u32 latency_frame_count;
} wasapi_sound_output_t;

gs_slot_array_decl( gs_audio_source_t );
gs_slot_array_decl( gs_audio_instance_data_t );

typedef struct audio_data_t
{
	// Main interface into OS sound hardware
	wasapi_sound_output_t output;

	// Other internal resource data, like audio resources
	s16* sample_out;				// Samples to actually write to hardware buffer
    u32 sample_count_to_output;	
    u32 samples_per_second;

    gs_slot_array( gs_audio_source_t ) 			sources;	// Raw source data
    gs_slot_array( gs_audio_instance_data_t ) 	instances; 	// Instanced data

} audio_data_t;

/*
	// Perhaps this is a default that could be entirely user defined? 
	typedef struct audio_instance_desc_t
	{
		gs_resource( gs_audio_source ) src;
		f32 volume;
		f64 play_position;
		b32 loop;
		b32 playing;
	} audio_instance_desc_t;
*/

/*
	// Load audio data from file, get back 'audio_source_t' struct or resource handle?

	gs_resource( gs_audio_source ) src = audio->load_audio_source( "source.wav" );

	audio_instance_desc_t desc = {0};
	desc.src = src;
	desc.volume = 2.f;
	desc.loop = false;
	desc.playing = true;
	desc.play_position = 0;

	// Instance of audio source
	gs_resource( gs_audio_instance ) instance = audio->play( void* );		// Will create an new instance internally and hold it until sound is done

	play_sound	
	resume_sound
	pause_sound
	stop_sound
		
	audio->play_sound( instance );
*/

void wasapi_init( gs_audio_i* audio )
{
	// Load WASAPI library
	HMODULE wasapi_lib = LoadLibraryA("ole32.dll");
    if(wasapi_lib)
    {
        CoCreateInstanceProc = (CoCreateInstance_ *)GetProcAddress(wasapi_lib, "CoCreateInstance");
        CoInitializeExProc = (CoInitializeEx_ *)GetProcAddress(wasapi_lib, "CoInitializeEx");
    }
    else
    {
        CoCreateInstanceProc = CoCreateInstanceStub;
        CoInitializeExProc = CoInitializeExStub;
    }

    audio_data_t* data = (audio_data_t*)(audio->data);
	wasapi_sound_output_t* output = &data->output;

    CoInitializeExProc(0, COINIT_SPEED_OVER_MEMORY);
    
    REFERENCE_TIME requested_sound_duration = REFTIMES_PER_SEC * 2;
    
    HRESULT result;
    
    result = CoCreateInstanceProc(&CLSID_MMDeviceEnumerator,
                                  0,
                                  CLSCTX_ALL,
                                  &IID_IMMDeviceEnumerator,
                                  (LPVOID *)(&output->device_enum));
    if(result == S_OK)
    {
        
        output->device_enum->lpVtbl->GetDefaultAudioEndpoint(output->device_enum,
                                                             eRender,
                                                             eConsole,
                                                             &output->device);
        if(result == S_OK)
        {
            
            result = output->device->lpVtbl->Activate(output->device,
                                                      &IID_IAudioClient,
                                                      CLSCTX_ALL,
                                                      0,
                                                      (void **)&output->audio_client);
            if(result == S_OK)
            {
                WAVEFORMATEX *wave_format = 0;
                
                output->audio_client->lpVtbl->GetMixFormat(output->audio_client, &wave_format);

                output->samples_per_second = 44100;//wave_format->nSamplesPerSec;
                WORD bits_per_sample = sizeof(s16)*8;
                WORD block_align = (bits_per_sample / 8) * output->channels;;
                DWORD average_bytes_per_second = block_align * output->samples_per_second;
                WORD cb_size = 0;
                
                WAVEFORMATEX new_wave_format = {
                    WAVE_FORMAT_PCM,
                    (WORD)output->channels,
                    output->samples_per_second,
                    average_bytes_per_second,
                    block_align,
                    bits_per_sample,
                    cb_size,
                };
                
                result = output->audio_client->lpVtbl->Initialize(output->audio_client,
                                                                  AUDCLNT_SHAREMODE_SHARED,
                                                                  AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
                                                                  requested_sound_duration,
                                                                  0,
                                                                  &new_wave_format,
                                                                  0);
                
                output->latency_frame_count = output->samples_per_second / SOUND_LATENCY_FPS;
                
                if(result == S_OK)
                {
                    
                    result = output->audio_client->lpVtbl->GetService(output->audio_client,
                                                                      &IID_IAudioRenderClient,
                                                                      (void **)&output->audio_render_client);
                    
                    if(result == S_OK)
                    {
                        output->audio_client->lpVtbl->GetBufferSize(output->audio_client, &output->buffer_frame_count);
                        
                        output->sound_buffer_duration = (REFERENCE_TIME)((f64)REFTIMES_PER_SEC *
                                                                         output->buffer_frame_count / output->samples_per_second);
                        
                        output->audio_client->lpVtbl->Start(output->audio_client);
                        
                        output->initialized = 1;
                    }
                    else
                    {
                        gs_println("WASAPI Error", "Request for audio render service failed.");
                        gs_assert( false );
                    }
                }
                else
                {
                    gs_println("WASAPI Error",
                                     "Audio client initialization failed.");
                    gs_assert( false );
                }
            }
            else
            {
                gs_println("WASAPI Error", "Could not activate audio device.");
                gs_assert( false );
            }
        }
        else
        {
            gs_println("WASAPI Error", "Default audio endpoint was not found.");
            gs_assert( false );
        }
    }
    else
    {
        gs_println("WASAPI Error", "Device enumerator retrieval failed.");
        gs_assert( false );
    }
}

void audio_query( gs_audio_i* audio )
{
	audio_data_t* data = (audio_data_t*)audio->data;

	data->sample_count_to_output = 0;
	u32 sound_padding_size;

	if ( SUCCEEDED( data->output.audio_client->lpVtbl->GetCurrentPadding( data->output.audio_client, &sound_padding_size ) ) )
	{
		data->samples_per_second = data->output.samples_per_second;
		data->sample_count_to_output = (u32)( data->output.latency_frame_count - sound_padding_size );

		if ( data->sample_count_to_output > data->output.latency_frame_count )
		{
			data->sample_count_to_output = data->output.latency_frame_count;
		}
	}

	// Set memory for output for frame count
	gs_for_range_i( data->output.buffer_frame_count )
	{
		data->sample_out[ i ] = 0;
	}
}

gs_result audio_init( gs_audio_i* audio )
{
	// Construct instance of render data
	struct audio_data_t* data = gs_malloc_init( struct audio_data_t );

	// Initialize sound output values
	data->output.channels = 2;
	data->output.samples_per_second = 48000;
	data->output.latency_frame_count = 48000;

    data->sources = gs_slot_array_new( gs_audio_source_t );
    data->instances = gs_slot_array_new( gs_audio_instance_data_t );

	// Allocate storage for samples output for hardware
	data->sample_out = gs_malloc( data->output.samples_per_second * sizeof(s16) * 2 );
	memset(data->sample_out, 0, data->output.samples_per_second * sizeof(s16) * 2);

	// Set data
	audio->data = data;

	wasapi_init( audio );

	// audio_query( audio );

	return gs_result_success;
}

gs_result audio_commit( gs_audio_i* audio )
{
	audio_data_t* __data = (audio_data_t*)audio->data;

	// Commit all audio to memory (fill sound buffer)
	u32 samples_to_write 			= __data->sample_count_to_output;
	s16* samples 					= __data->sample_out;
	wasapi_sound_output_t* output 	= &__data->output;

	if ( samples_to_write )
	{
		BYTE* data = 0;
		DWORD flags = 0;

		output->audio_render_client->lpVtbl->GetBuffer( output->audio_render_client, samples_to_write, &data );
		if ( data )
		{
			s16* dst = (s16*)data;
			s16* src = samples;

			gs_for_range_i( samples_to_write )
			{
				*dst++ = *src++; // left sample
				*dst++ = *src++; // right sample
			}
		}

		output->audio_render_client->lpVtbl->ReleaseBuffer( output->audio_render_client, samples_to_write, flags );
	}

	audio_query( audio );

	return gs_result_success;
}

// Default audio upate function
gs_result audio_update( gs_audio_i* audio ) 
{
	// For the update, we're going to iterate through all contiguous instance data, check whether or not it's playing, 
	// Then we'll just play the shiz
	audio_data_t* __data = (audio_data_t*)audio->data;

	for ( u32 i = 0; i < gs_slot_array_size( __data->instances ); ++i )
	{
		gs_audio_instance_data_t* inst = &__data->instances.data[ i ];

		// Get raw audio source from instance
		gs_audio_source_t* src = gs_slot_array_get_ptr( __data->sources, inst->src.id );

		// Easy out if the instance is not playing currently or the source is invalid
		if ( !inst->playing || !src ) {
			continue;
		}

		s16* sample_out = __data->sample_out;
		s16* samples = (s16*)src->samples;

		u64 samples_to_write = __data->sample_count_to_output;
		f64 sample_volume = inst->volume;

		// Write to channels
		for ( u64 write_sample = 0; write_sample < samples_to_write; ++write_sample )
		{
			s32 channels = src->channels;
			f64 start_sample_position = inst->sample_position;
			s16 start_left_sample;
			s16 start_right_sample;

			// Not sure about this line of code...
			f64 target_sample_position = start_sample_position + (f64)channels * (f64)1.f;

			if ( target_sample_position >= src->sample_count )
			{
				target_sample_position -= src->sample_count;
			}

			s16 target_left_sample;
			s16 target_right_sample;

			{
				u64 left_idx = (u64)start_sample_position;
				if ( channels > 1 )
				{
					left_idx &= ~((u64)(0x01));
				}
				u64 right_idx = left_idx + (channels - 1);

				s16 first_left_sample = samples[left_idx];
				s16 first_right_sample = samples[right_idx];
				s16 second_left_sample = samples[left_idx + channels];
				s16 second_right_sample = samples[right_idx + channels];

				start_left_sample = (s16)(first_left_sample + (second_left_sample - first_left_sample) * (start_sample_position / channels - (u64)(start_sample_position / channels)));
                start_right_sample = (s16)(first_right_sample + (second_right_sample - first_right_sample) * (start_sample_position / channels - (u64)(start_sample_position / channels)));
			}

			{
                u64 left_index = (u64)target_sample_position;
                if(channels > 1)
                {
                    left_index &= ~((u64)(0x01));
                }
                u64 right_index = left_index + (channels - 1);
                
                s16 first_left_sample = samples[left_index];
                s16 first_right_sample = samples[right_index];
                s16 second_left_sample = samples[left_index + channels];
                s16 second_right_sample = samples[right_index + channels];
                
                target_left_sample = (s16)(first_left_sample + (second_left_sample - first_left_sample) * (target_sample_position / channels - (u64)(target_sample_position / channels)));
                target_right_sample = (s16)(first_right_sample + (second_right_sample - first_right_sample) * (target_sample_position / channels - (u64)(target_sample_position / channels)));
            }

            s16 left_sample = (s16)((((s64)start_left_sample + (s64)target_left_sample) / 2) * sample_volume);
            s16 right_sample = (s16)((((s64)start_right_sample + (s64)target_right_sample) / 2) * sample_volume);

            *sample_out++ += left_sample;  // Left
            *sample_out++ += right_sample; // Right

            // Possibly need fixed sampling instead
            inst->sample_position = target_sample_position;

            // Loop sound if necessary
            if( inst->sample_position >= (u32)((f32)src->sample_count * 0.95f) ||  inst->sample_position >= src->sample_count - channels - 1 )
            {
                if( inst->loop )
                {
                    // inst->sample_position -= src->sample_count;
                    inst->sample_position = 0;
                }
                else
                {
                	// Need to destroy the instance at this point...
                    inst->playing = false;
                    inst->sample_position = 0;
                    break;
                }
            }

		}		
	}

	return gs_result_success;
}

gs_result audio_shutdown( gs_audio_i* audio )
{
	return gs_result_success;
}

b32 load_ogg_data( const char* file_name, gs_audio_source_t* src )
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// typedef struct gs_audio_source_t
	// {
	// 	s32 channels;
	// 	s32 sample_rate;
	// 	void* samples;
	// 	s32 sample_count;
	// } gs_audio_source_t;

	// Init OGG
	{
		src->sample_count = stb_vorbis_decode_filename( file_name, &src->channels, 
			&src->sample_rate, (s16**)&src->samples );

		if ( !src->samples || src->sample_count == -1 )
		{
			src->samples = NULL; 
			gs_println( "WARNING: Could not load .ogg file: %s", file_name );
			return false; 
		} 

		src->sample_count *= src->channels;

		return true;
	}

	return false;
}

/*============================================================
// Audio Source
============================================================*/

gs_resource( gs_audio_source ) load_audio_source_from_file( const char* file_name )
{
	gs_audio_source_t source = {0};
	gs_resource( gs_audio_source ) handle = gs_resource_invalid( gs_audio_source );
	b32 load_successful = false;

	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	audio_data_t* __data = (audio_data_t*)audio->data;

	if ( !platform->file_exists( file_name ) ) {
		gs_println( "WARNING: Could not open file: %s", file_name );
		return handle;
	}

	// Lower case and grab file ext.
	char file_ext[64] = {0};
	gs_util_str_to_lower( file_name, file_ext, sizeof(file_ext) );
	platform->file_extension( file_ext, sizeof(file_ext), file_ext );

	// Load OGG data
	if ( gs_string_compare_equal( file_ext, "ogg" ) ) {
		load_successful = load_ogg_data( file_name, &source );
	}

	// Load raw source into memory and return handle id
	if ( load_successful )
	{
		gs_println( "SUCCESS: Audio source loaded: %s", file_name );
		u32 id = gs_slot_array_insert( __data->sources, source );
		handle.id = id;
	}
	else
	{
		gs_println( "WARNING: Could not load audio source data: %s", file_name );
	}

	return handle;
}

/*============================================================
// Audio Instance Data
============================================================*/

gs_resource( gs_audio_instance ) play( gs_audio_instance_data_t inst )
{
	gs_resource( gs_audio_instance ) inst_h = gs_resource_invalid( gs_audio_instance );

	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	audio_data_t* __data = (audio_data_t*)audio->data;

	// Verify that source is valid first
	if ( gs_slot_array_handle_valid( __data->sources, inst.src.id ) )
	{
		u32 id = gs_slot_array_insert( __data->instances, inst );
		inst_h.id = id;
	}

	return inst_h;
}

void pause( gs_resource( gs_audio_instance ) inst_h )
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	audio_data_t* __data = (audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr( __data->instances, inst_h.id );
	if ( inst )
	{
		inst->playing = false;
	}
}

void resume( gs_resource( gs_audio_instance ) inst_h )
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	audio_data_t* __data = (audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr( __data->instances, inst_h.id );
	if ( inst )
	{
		inst->playing = true;
	}
}

void restart( gs_resource( gs_audio_instance ) inst_h )
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	audio_data_t* __data = (audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr( __data->instances, inst_h.id );
	if ( inst )
	{
		inst->playing = true;
		inst->sample_position = 0;
	}
}

void set_instance_data( gs_resource( gs_audio_instance ) inst_h, gs_audio_instance_data_t data )
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	audio_data_t* __data = (audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr( __data->instances, inst_h.id );
	if ( inst )
	{
		*inst = data;
	}
}

f32 get_volume( gs_resource( gs_audio_instance ) inst_h )
{
	 gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	audio_data_t* __data = (audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr( __data->instances, inst_h.id );
	if ( inst )
	{
		return inst->volume;
	}

	return 0.f;
}

void set_volume( gs_resource( gs_audio_instance ) inst_h, f32 vol )
{
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	audio_data_t* __data = (audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr( __data->instances, inst_h.id );
	if ( inst )
	{
		inst->volume = vol;
	}
}

void stop( gs_resource( gs_audio_instance ) inst_h )
{
	// Should actually destroy a sound if it's not persistent
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	audio_data_t* __data = (audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr( __data->instances, inst_h.id );
	if ( inst )
	{
		inst->playing = false;
	}
}

/*============================================================
// Audio API Construction
============================================================*/

struct gs_audio_i* gs_audio_construct()
{
	struct gs_audio_i* audio = gs_malloc_init( struct gs_audio_i );

	// Null out all data
	audio->data = NULL;
	audio->user_data = NULL;

	// Audio Init/De-Init Functions
	audio->init 		= &audio_init;
	audio->update 		= &audio_update;
	audio->shutdown 	= &audio_shutdown;
	audio->commit 		= &audio_commit;

	audio->load_audio_source_from_file 	= &load_audio_source_from_file;
	audio->play 						= &play;
	audio->pause 						= &pause;
	audio->resume 						= &resume;
	audio->restart 						= &restart;
	audio->stop 						= &stop;
	audio->get_volume 					= &get_volume;
	audio->set_volume 					= &set_volume;
	audio->set_instance_data 			= &set_instance_data;

	return audio;
}
