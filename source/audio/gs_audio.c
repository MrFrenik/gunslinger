#include "audio/gs_audio.h"

// Default audio upate function (taken from Ryan Fluery's win32 app template)
gs_result __gs_audio_update_internal( struct gs_audio_i* audio ) 
{
	// For the update, we're going to iterate through all contiguous instance data, check whether or not it's playing, 
	// Then we'll just play the shiz
	gs_audio_data_t* __data = (gs_audio_data_t*)audio->data;

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
            if( inst->sample_position >= src->sample_count - channels - 1 )
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
