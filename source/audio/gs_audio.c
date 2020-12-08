#include "audio/gs_audio.h"
#include "base/gs_engine.h"
#include "platform/gs_platform.h"

#include "stb/stb_vorbis.c"
#include "dr_libs/dr_wav.h"
#include "dr_libs/dr_mp3.h"

// typedef struct gs_audio_source_t
// {
// 	s32 channels;
// 	s32 sample_rate;
// 	void* samples;
// 	s32 sample_count;
// } gs_audio_source_t;

b32 __gs_load_ogg_data(const char* file_name, gs_audio_source_t* src)
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Init OGG
	{
		src->sample_count = stb_vorbis_decode_filename(file_name, &src->channels, 
			&src->sample_rate, (s16**)&src->samples);

		if (!src->samples || src->sample_count == -1)
		{
			src->samples = NULL; 
			gs_println("WARNING: Could not load .ogg file: %s", file_name);
			return false; 
		} 

		src->sample_count *= src->channels;

		return true;
	}

	return false;
}

b32 __gs_load_wav_data(const char* file_name, gs_audio_source_t* src)
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	u64 total_pcm_frame_count = 0;
	src->samples = drwav_open_file_and_read_pcm_frames_s16(file_name, (u32*)&src->channels, (u32*)&src->sample_rate, 
		&total_pcm_frame_count, NULL);

	if (!src->samples) {
		src->samples = NULL; 
		gs_println("WARNING: Could not load .ogg file: %s", file_name);
		return false; 
	}

	src->sample_count = total_pcm_frame_count * src->channels;

	return true;
}

b32 __gs_load_mp3_data(const char* file_name, gs_audio_source_t* src)
{
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	// Decode entire mp3 
	u64 total_pcm_frame_count = 0;
	drmp3_config cfg = {0};
	src->samples = drmp3_open_file_and_read_pcm_frames_s16(file_name, &cfg, &total_pcm_frame_count, NULL);

	if (!src->samples) {
		src->samples = NULL; 
		gs_println("WARNING: Could not load .ogg file: %s", file_name);
		return false; 
	}

	src->channels = cfg.channels;
	src->sample_rate = cfg.sampleRate;
	src->sample_count = total_pcm_frame_count * src->channels;

	return true;
}

// Default audio upate function (taken from Ryan Fluery's win32 app template)
gs_result __gs_audio_update_internal(struct gs_audio_i* audio) 
{
	// For the update, we're going to iterate through all contiguous instance data, check whether or not it's playing, 
	// Then we'll just play the shiz
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;

	if (!__data) 
	{
		return gs_result_incomplete;
	}

	for (u32 i = 0; i < gs_slot_array_size(__data->instances); ++i)
	{
		gs_audio_instance_data_t* inst = &__data->instances.data[ i ];

		// Get raw audio source from instance
		gs_audio_source_t* src = gs_resource_cache_get_ptr(audio->audio_cache, inst->src.id);

		// Easy out if the instance is not playing currently or the source is invalid
		if (!inst->playing || !src) {
			continue;
		}

		s16* sample_out = __data->sample_out;
		s16* samples = (s16*)src->samples;

		u64 samples_to_write = __data->sample_count_to_output;
		f64 sample_volume = inst->volume;

		// Write to channels
		for (u64 write_sample = 0; write_sample < samples_to_write; ++write_sample)
		{
			s32 channels = src->channels;
			f64 start_sample_position = inst->sample_position;
			s16 start_left_sample;
			s16 start_right_sample;

			// Not sure about this line of code...
			f64 target_sample_position = start_sample_position + (f64)channels * (f64)1.f; // pitch

			if (target_sample_position >= src->sample_count)
			{
				target_sample_position -= src->sample_count;
			}

			s16 target_left_sample;
			s16 target_right_sample;

			{
				u64 left_idx = (u64)start_sample_position;
				if (channels > 1)
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
            if(inst->sample_position >= src->sample_count - channels - 1)
            {
                if(inst->loop)
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

/*============================================================
// Audio Source
============================================================*/

// This will allocate memory for user.
gs_resource(gs_audio_source_t) __gs_load_audio_source_from_file(const char* file_name)
{
	gs_audio_source_t source = gs_default_val();
	b32 load_successful = false;

	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_platform_i* platform = gs_engine_instance()->ctx.platform;

	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;

	if (!platform->file_exists(file_name)) {
		gs_println("WARNING: Could not open file: %s", file_name);
		return gs_resource_invalid(gs_audio_source_t);
	}

	// Lower case and grab file ext.
	char file_ext[64] = {0};
	gs_util_str_to_lower(file_name, file_ext, sizeof(file_ext));
	platform->file_extension(file_ext, sizeof(file_ext), file_ext);

	// Load OGG data
	if (gs_string_compare_equal(file_ext, "ogg")) {
		load_successful = __gs_load_ogg_data(file_name, &source);
	}

	if (gs_string_compare_equal(file_ext, "wav")) {
		load_successful = __gs_load_wav_data(file_name, &source);	
	}

	if (gs_string_compare_equal(file_ext, "mp3")) {
		load_successful = __gs_load_mp3_data(file_name, &source);	
	}

	// Load raw source into memory and return handle id
	if (load_successful)
	{
		gs_println("SUCCESS: Audio source loaded: %s", file_name);

		// Add to resource cache
		gs_resource(gs_audio_source_t) handle = gs_default_val();
		handle.id = gs_resource_cache_insert(audio->audio_cache, source);
		return handle;
	}
	else
	{
		gs_println("WARNING: Could not load audio source data: %s", file_name);
		return gs_resource_invalid(gs_audio_source_t);
	}

	return gs_resource_invalid(gs_audio_source_t);
}

/*============================================================
// Audio Instance Data
============================================================*/

gs_resource(gs_audio_instance_t) __gs_audio_construct_instance(gs_audio_instance_data_t inst)
{
	gs_resource(gs_audio_instance_t) inst_h = gs_resource_invalid(gs_audio_instance_t);

	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	u32 id = gs_slot_array_insert(__data->instances, inst);
	inst_h.id = id;
	return inst_h;
}

void __gs_audio_play_source(gs_resource(gs_audio_source_t) src, f32 volume)
{
	// Construct instance data from source
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_instance_data_t inst = gs_audio_instance_data_new(src);
	inst.volume = gs_clamp(volume, audio->min_audio_volume, audio->max_audio_volume);
	inst.persistent = false;
	gs_resource(gs_audio_instance_t) inst_h = audio->construct_instance(inst);
	audio->play(inst_h);
}

// Essentially just restarts the audio instance
void __gs_audio_play(gs_resource(gs_audio_instance_t) inst_h)
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	if (inst)
	{
		inst->playing = true;
	}
}

void __gs_audio_pause(gs_resource(gs_audio_instance_t) inst_h)
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	if (inst)
	{
		inst->playing = false;
	}
}

void __gs_audio_restart(gs_resource(gs_audio_instance_t) inst_h)
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	if (inst)
	{
		inst->playing = true;
		inst->sample_position = 0;
	}
}

void __gs_audio_set_instance_data(gs_resource(gs_audio_instance_t) inst_h, gs_audio_instance_data_t data)
{
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	if (inst)
	{
		*inst = data;
	}
}

gs_audio_instance_data_t __gs_audio_get_instance_data(gs_resource(gs_audio_instance_t) inst_h)
{
	gs_audio_instance_data_t data = gs_default_val();	
	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	if (inst)
	{
		data = *inst;
	}
	return data;
}

f32 __gs_audio_get_volume(gs_resource(gs_audio_instance_t) inst_h)
{
	 gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	if (inst)
	{
		return inst->volume;
	}

	return 0.f;
}

void __gs_audio_set_volume(gs_resource(gs_audio_instance_t) inst_h, f32 vol)
{
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	if (inst)
	{
		inst->volume = gs_clamp(vol, audio->min_audio_volume, audio->max_audio_volume);
	}
}

void __gs_audio_stop(gs_resource(gs_audio_instance_t) inst_h)
{
	// Should actually destroy a sound if it's not persistent
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	if (inst)
	{
		inst->playing = false;
		inst->sample_position = 0;
	}
}

b32 __gs_audio_is_playing(gs_resource(gs_audio_instance_t) inst_h)
{
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	gs_audio_instance_data_t* inst = gs_slot_array_get_ptr(__data->instances, inst_h.id);
	return (inst && inst->playing);
}

void __gs_audio_get_runtime(gs_audio_source_t* src, s32* _minutes, s32* _seconds)
{
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;
	if (src)
	{
		// Calculate total length in seconds
		f64 total_seconds = ((f32)src->sample_count / (f32)src->sample_rate) / src->channels;
		s32 seconds = (s32)(fmodf(total_seconds, 60.f));
		s32 minutes = (s32)(total_seconds / 60.f);	

		if (_minutes)
		{
			*_minutes = minutes;
		}

		if (_seconds)
		{
			*_seconds = seconds;
		}
	}
}

s32 __gs_get_sample_count(gs_resource(gs_audio_source_t) src)
{
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
 	gs_audio_source_t* sp = gs_resource_cache_get_ptr(audio->audio_cache, src.id);
 	if (sp)
 	{
 		return sp->sample_count; 
 	}
 	return 0;
}

s32 __gs_get_sample_rate(gs_resource(gs_audio_source_t) src)
{
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
 	gs_audio_source_t* sp = gs_resource_cache_get_ptr(audio->audio_cache, src.id);
 	if (sp)
 	{
 		return sp->sample_rate; 
 	}
 	return 0;
}

s32 __gs_get_num_channels(gs_resource(gs_audio_source_t) src)
{
 	gs_audio_i* audio = gs_engine_instance()->ctx.audio;
 	gs_audio_source_t* sp = gs_resource_cache_get_ptr(audio->audio_cache, src.id);
 	if (sp)
 	{
 		return sp->channels; 
 	}
 	return 0;
}

void __gs_audio_convert_to_runtime(s32 sample_count, s32 sample_rate, 
	s32 num_channels, s32 position, s32* minutes_out, s32* seconds_out)
{
	// Calculate total length in seconds
	f64 frac = (f64)position / (f64)sample_count;
	f64 total_seconds = ((f64)sample_count / (f64)sample_rate) / num_channels;
	total_seconds = total_seconds * frac;
	s32 seconds = (s32)(fmodf(total_seconds, 60.f));
	s32 minutes = (s32)(total_seconds / 60.f);	

	if (minutes_out)
	{
		*minutes_out = minutes;
	}

	if (seconds_out)
	{
		*seconds_out = seconds;
	}
}

struct gs_audio_i* __gs_audio_construct()
{
	struct gs_audio_i* audio = gs_malloc_init(struct gs_audio_i);
    struct gs_audio_data_t* data = gs_malloc_init(struct gs_audio_data_t);

    data->internal = NULL;
    data->instances = gs_slot_array_new(gs_audio_instance_data_t);

    // Set data
    audio->user_data = NULL;
    audio->data = data;

    // Default audio max/min
    audio->max_audio_volume = 1.f;
    audio->min_audio_volume = 0.f;

	audio->audio_cache = gs_resource_cache_new(gs_audio_source_t);

	// Default internals
	__gs_audio_set_default_functions(audio);

	return audio;
}

void __gs_audio_set_default_functions(struct gs_audio_i* audio)
{
	audio->update 						= &__gs_audio_update_internal;
	audio->load_audio_source_from_file 	= &__gs_load_audio_source_from_file;
	audio->construct_instance 			= &__gs_audio_construct_instance;
	audio->play 						= &__gs_audio_play;
	audio->play_source 					= &__gs_audio_play_source;
	audio->pause 						= &__gs_audio_pause;
	audio->restart 						= &__gs_audio_restart;
	audio->stop 						= &__gs_audio_stop;
	audio->get_volume 					= &__gs_audio_get_volume;
	audio->set_volume 					= &__gs_audio_set_volume;
	audio->is_playing 					= &__gs_audio_is_playing;
	audio->set_instance_data 			= &__gs_audio_set_instance_data;
	audio->get_instance_data 			= &__gs_audio_get_instance_data;
	audio->get_runtime 					= &__gs_audio_get_runtime;
	audio->convert_to_runtime 			= &__gs_audio_convert_to_runtime;
	audio->get_sample_count 			= &__gs_get_sample_count;
	audio->get_sample_rate 				= &__gs_get_sample_rate;
	audio->get_num_channels 			= &__gs_get_num_channels;

	gs_audio_construct_internal(audio);
}
