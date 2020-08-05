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

    gs_audio_data_t* data = (gs_audio_data_t*)(audio->data);
	wasapi_sound_output_t* output = data->internal;

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
	gs_audio_data_t* data = (gs_audio_data_t*)audio->data;
	wasapi_sound_output_t* output = (wasapi_sound_output_t*)data->internal;

	data->sample_count_to_output = 0;
	u32 sound_padding_size;

	if ( SUCCEEDED( output->audio_client->lpVtbl->GetCurrentPadding( output->audio_client, &sound_padding_size ) ) )
	{
		data->samples_per_second = output->samples_per_second;
		data->sample_count_to_output = (u32)( output->latency_frame_count - sound_padding_size );

		if ( data->sample_count_to_output > output->latency_frame_count )
		{
			data->sample_count_to_output = output->latency_frame_count;
		}
	}

	s16* samples = (s16*)(data->sample_out);

	// Set memory for output for frame count
	gs_for_range_i( output->buffer_frame_count )
	{
		samples[ i ] = 0;
	}
}

gs_result audio_init( gs_audio_i* audio )
{
	// Construct instance of render data
	struct gs_audio_data_t* data = audio->data;
	data->internal = gs_malloc_init( struct wasapi_sound_output_t );

	wasapi_sound_output_t* output = (wasapi_sound_output_t*)data->internal;

	// Initialize sound output values
	output->channels = 2;
	output->samples_per_second = 48000;
	output->latency_frame_count = 48000;

	// Allocate storage for samples output for hardware
	data->sample_out = gs_malloc( output->samples_per_second * sizeof(s16) * 2 );
	memset(data->sample_out, 0, output->samples_per_second * sizeof(s16) * 2);

	wasapi_init( audio );

	return gs_result_success;
}

gs_result audio_commit( gs_audio_i* audio )
{
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;

	// Commit all audio to memory (fill sound buffer)
	u32 samples_to_write 			= __data->sample_count_to_output;
	s16* samples 					= __data->sample_out;
	wasapi_sound_output_t* output 	= __data->internal;

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

gs_result audio_shutdown( gs_audio_i* audio )
{
	// NOTE(john): Free resources and stuff...
	return gs_result_success;
}

/*===================================================================
// Audio API Construction (fill out specific internal for custom API)
====================================================================*/

void gs_audio_construct_internal( struct gs_audio_i* audio )
{
	// Audio Init/De-Init Functions
	audio->init 		= &audio_init;
	audio->shutdown 	= &audio_shutdown;
	audio->commit 		= &audio_commit;
}
