
/*================================================================
    * Copyright: 2020 John Jackson 
    * File: gs_audio_impl.h
    All Rights Reserved
=================================================================*/

#ifndef __GS_AUDIO_IMPL_H__
#define __GS_AUDIO_IMPL_H__

// Define default platform implementation if certain platforms are enabled
#if (defined GS_AUDIO_IMPL_MINIAUDIO)
    #define GS_AUDIO_IMPL_DEFAULT
#endif

/*=============================
// Default Impl
=============================*/

#ifdef GS_AUDIO_IMPL_DEFAULT

// Includes
#include "../external/stb/stb_vorbis.c"
#include "../external/dr_libs/dr_wav.h"
#include "../external/dr_libs/dr_mp3.h"

/* Audio Create, Destroy, Init, Shutdown, Submit */
gs_audio_i* gs_audio_create()
{
    // Construct new audio interface
    gs_audio_i* audio = gs_malloc_init(gs_audio_i);
    /* Audio source data cache */
    audio->sources = gs_slot_array_new(gs_audio_source_t);
    /* Audio instance data cache */
    audio->instances = gs_slot_array_new(gs_audio_instance_t);
    /* Max global volume setting */
    audio->max_audio_volume = 1.f;
    /* Min global volume setting */
    audio->min_audio_volume = 0.f;
    /* Set user data to null */
    audio->user_data = NULL;

    return audio;
}

void gs_audio_destroy(gs_audio_i* audio)
{
    // Release all relevant memory
    if (audio)
    {
        gs_slot_array_free(audio->sources);
        gs_slot_array_free(audio->instances);
        gs_free(audio);
        audio = NULL;
    }
}

/* Resource Loading */
bool32_t gs_audio_load_ogg_data_from_file
(
    const char* file_path, 
    int32_t* sample_count, 
    int32_t* channels,
    int32_t* sample_rate, 
    void** samples
)
{
    *sample_count = stb_vorbis_decode_filename(file_path, channels, 
        sample_rate, (s16**)samples);

    if (!*samples || *sample_count == -1)
    {
        *samples = NULL; 
        gs_println("WARNING: Could not load .ogg file: %s", file_path);
        return false; 
    } 

    *sample_count *= *channels;

    return true;
}

bool32_t gs_audio_load_wav_data_from_file
(
    const char* file_path, 
    int32_t* sample_count, 
    int32_t* channels, 
    int32_t* sample_rate, 
    void** samples
)
{
    uint64_t total_pcm_frame_count = 0;
    *samples = drwav_open_file_and_read_pcm_frames_s16(
        file_path, (uint32_t*)channels, (uint32_t*)sample_rate, 
        &total_pcm_frame_count, NULL);

    if (!*samples) {
        *samples = NULL; 
        gs_println("WARNING: Could not load .ogg file: %s", file_path);
        return false; 
    }

    *sample_count = total_pcm_frame_count * *channels;

    return true;
}

bool32_t gs_audio_load_mp3_data_from_file
(
    const char* file_path, 
    int32_t* sample_count, 
    int32_t* channels, 
    int32_t* sample_rate, 
    void** samples
)
{
    // Decode entire mp3 
    uint64_t total_pcm_frame_count = 0;
    drmp3_config cfg = gs_default_val();
    *samples = drmp3_open_file_and_read_pcm_frames_s16(
        file_path, &cfg, &total_pcm_frame_count, NULL);

    if (!*samples) {
        *samples = NULL; 
        gs_println("WARNING: Could not load .ogg file: %s", file_path);
        return false; 
    }

    *channels = cfg.channels;
    *sample_rate = cfg.sampleRate;
    *sample_count = total_pcm_frame_count * *channels;

    return true;
}

/* Audio create source */
gs_handle(gs_audio_source_t) gs_audio_load_from_file(const char* file_path)
{
    gs_audio_i* audio = gs_engine_subsystem(audio);
    gs_audio_source_t src = gs_default_val();
    gs_handle(gs_audio_source_t) handle = gs_handle_invalid(gs_audio_source_t);
    bool32_t load_successful = false;

    if(!gs_platform_file_exists(file_path)) 
    {
        gs_println("WARNING: Could not open file: %s", file_path);
        return handle;
    }

    char ext[64] = gs_default_val();
    gs_util_str_to_lower(file_path, ext, sizeof(ext));
    gs_platform_file_extension(ext, sizeof(ext), ext);

    // Load OGG data
    if (gs_string_compare_equal(ext, "ogg"))
    {

        load_successful = gs_audio_load_ogg_data_from_file (
            file_path, 
            &src.sample_count, 
            &src.channels,
            &src.sample_rate, 
            &src.samples
        );
    }

    // Load WAV data
    if (gs_string_compare_equal(ext, "wav"))
    {
        load_successful = gs_audio_load_wav_data_from_file (
            file_path, 
            &src.sample_count, 
            &src.channels,
            &src.sample_rate, 
            &src.samples
        );
    }

    if (gs_string_compare_equal(ext, "mp3"))
    {
        load_successful = gs_audio_load_mp3_data_from_file (
            file_path, 
            &src.sample_count, 
            &src.channels,
            &src.sample_rate, 
            &src.samples
        );
    }

    // Load raw source into memory and return handle id
    if (load_successful)
    {
        gs_println("SUCCESS: Audio source loaded: %s", file_path);

        // Add to resource cache
        handle.id = gs_slot_array_insert(audio->sources, src);
    }
    else
    {
        gs_println("WARNING: Could not load audio source data: %s", file_path);
    }

    return handle;
}

/* Audio create instance */
gs_handle(gs_audio_instance_t) gs_audio_instance_create(gs_audio_instance_decl_t* decl)
{
    gs_audio_i* audio = gs_engine_subsystem(audio);
    return gs_handle_create(gs_audio_instance_t, gs_slot_array_insert(audio->instances, *decl));    
}

/* Audio play instance data */
void gs_audio_play_source(gs_handle(gs_audio_source_t) src, float volume)
{
    // Construct instance data from source and play
    gs_audio_i* audio = gs_engine_subsystem(audio);
    gs_audio_instance_decl_t decl = gs_default_val();
    decl.volume = gs_clamp(volume, audio->min_audio_volume, audio->max_audio_volume);
    decl.persistent = false;
    gs_handle(gs_audio_instance_t) inst = gs_audio_instance_create(&decl);
    gs_audio_play(inst);
}

// Helper macros
#define __gs_audio_inst_valid(INST)\
    gs_slot_array_handle_valid(gs_engine_subsystem(audio)->instances, INST.id)

#define __gs_audio_src_valid(SRC)\
    gs_slot_array_handle_valid(gs_engine_subsystem(audio)->sources, SRC.id)

void gs_audio_play(gs_handle(gs_audio_instance_t) inst)
{
    if (__gs_audio_inst_valid(inst)) {
        gs_slot_array_getp(gs_engine_subsystem(audio)->instances, inst.id)->playing = true;
    }
}

void gs_audio_pause(gs_handle(gs_audio_instance_t) inst)
{
    if (__gs_audio_inst_valid(inst)) {
        gs_slot_array_getp(gs_engine_subsystem(audio)->instances, inst.id)->playing = false;
    }
}

void gs_audio_stop(gs_handle(gs_audio_instance_t) inst)
{
    if (__gs_audio_inst_valid(inst)) {
        gs_audio_instance_t* ip = gs_slot_array_getp(gs_engine_subsystem(audio)->instances, inst.id);
        ip->playing = false;
        ip->sample_position = 0;
    }
}

void gs_audio_restart(gs_handle(gs_audio_instance_t) inst)
{
    if (__gs_audio_inst_valid(inst)) {
        gs_slot_array_getp(gs_engine_subsystem(audio)->instances, inst.id)->sample_position = 0;
    }
}

bool32_t gs_audio_is_playing(gs_handle(gs_audio_instance_t) inst)
{
    if (__gs_audio_inst_valid(inst)) {
        return (gs_slot_array_getp(gs_engine_subsystem(audio)->instances, inst.id)->playing);
    }
    return false;
}

/* Audio instance data */
void gs_audio_set_instance_data(gs_handle(gs_audio_instance_t) inst, gs_audio_instance_decl_t decl)
{
    if (__gs_audio_inst_valid(inst)) {
        *gs_slot_array_getp(gs_engine_subsystem(audio)->instances, inst.id) = decl;
    }
}

gs_audio_instance_decl_t gs_audio_get_instance_data(gs_handle(gs_audio_instance_t) inst)
{
    if (__gs_audio_inst_valid(inst)) {
        return gs_slot_array_get(gs_engine_subsystem(audio)->instances, inst.id);
    }
    gs_audio_instance_decl_t decl = gs_default_val();
    return decl;
}

float gs_audio_get_volume(gs_handle(gs_audio_instance_t) inst)
{
    if (__gs_audio_inst_valid(inst)) {
        return gs_slot_array_getp(gs_engine_subsystem(audio)->instances, inst.id)->volume;
    }
    return 0.f;
}

void gs_audio_set_volume(gs_handle(gs_audio_instance_t) inst, float volume)
{
    if (__gs_audio_inst_valid(inst)) {
        gs_slot_array_getp(gs_engine_subsystem(audio)->instances, inst.id)->volume = volume;
    }
}

/* Audio source data */
gs_audio_source_t* gs_audio_get_source_data(gs_handle(gs_audio_source_t) src)
{
    if (__gs_audio_src_valid(src)) {
        return gs_slot_array_getp(gs_engine_subsystem(audio)->sources, src.id);
    }
    return NULL;
}

void gs_audio_get_runtime(gs_handle(gs_audio_source_t) src, int32_t* minutes_out, int32_t* seconds_out)
{
    if (__gs_audio_src_valid(src)) {
        gs_audio_i* audio = gs_engine_subsystem(audio);
        gs_audio_source_t* sp = gs_slot_array_getp(audio->sources, src.id);
        if (sp)
        {
            // Calculate total length in seconds
            float64_t total_seconds = ((float32_t)sp->sample_count / (float32_t)sp->sample_rate) / sp->channels;
            int32_t seconds = (int32_t)(fmodf(total_seconds, 60.f));
            int32_t minutes = (int32_t)(total_seconds / 60.f);  

            if (minutes_out) {
                *minutes_out = minutes;
            }

            if (seconds_out) {
                *seconds_out = seconds;
            }
        }
    }
}

void gs_audio_convert_to_runtime(int32_t sample_count, int32_t sample_rate, int32_t num_channels, int32_t position, int32_t* minutes_out, int32_t* seconds_out)
{
    // Calculate total length in seconds
    float64_t frac = (float64_t)position / (float64_t)sample_count;
    float64_t total_seconds = ((float64_t)sample_count / (float64_t)sample_rate) / num_channels;
    total_seconds = total_seconds * frac;
    int32_t seconds = (int32_t)(fmodf(total_seconds, 60.f));
    int32_t minutes = (int32_t)(total_seconds / 60.f);  

    if (minutes_out) {
        *minutes_out = minutes;
    }

    if (seconds_out) {
        *seconds_out = seconds;
    }
}

int32_t gs_audio_get_sample_count(gs_handle(gs_audio_source_t) src)
{
    if (__gs_audio_src_valid(src)) {
        return gs_slot_array_getp(gs_engine_subsystem(audio)->sources, src.id)->sample_count;
    }
    return 0;
}

int32_t gs_audio_get_sample_rate(gs_handle(gs_audio_source_t) src)
{
    if (__gs_audio_src_valid(src)) {
        return gs_slot_array_getp(gs_engine_subsystem(audio)->sources, src.id)->sample_rate;
    }
    return 0;
}

int32_t gs_audio_get_num_channels(gs_handle(gs_audio_source_t) src)
{
    if (__gs_audio_src_valid(src)) {
        return gs_slot_array_getp(gs_engine_subsystem(audio)->sources, src.id)->channels;
    }
    return 0;
}

#undef GS_AUDIO_IMPL_DEFAULT
#endif // GS_AUDIO_IMPL_DEFAULT

/*=============================
// Miniaudio Impl
=============================*/

#ifdef GS_AUDIO_IMPL_MINIAUDIO

#define MINIAUDIO_IMPLEMENTATION
#include "../external/miniaudio/miniaudio.h"

typedef struct miniaudio_data_t
{
    ma_context context;
    ma_device device;
    ma_device_config device_config;
    ma_mutex lock;
} miniaudio_data_t;

void ma_audio_commit(ma_device* device, void* output, const void* input, ma_uint32 frame_count)
{
    gs_audio_i* audio = gs_engine_subsystem(audio);
    miniaudio_data_t* ma = (miniaudio_data_t*)audio->user_data;
    memset(output, 0, frame_count * device->playback.channels * ma_get_bytes_per_sample(device->playback.format));

    // Only destroy 32 at a time
    u32 destroy_count = 0;
    gs_handle(gs_audio_instance_t) handles_to_destroy[32];

    if (!audio->instances) 
        return;

    // Mutex not working for pushing samples back. Need to copy sample data OVER at a synced position.
    // Add sample data into byte buffer to push back, but this has to be done to sync with audio
    // thread so that it's consistent and smooth feeding.
    ma_mutex_lock(&ma->lock);
    {
        for (u32 i = 0; i < gs_slot_array_size(audio->instances); ++i)
        {
            gs_audio_instance_t* inst = &audio->instances->data[i];

            // Get raw audio source from instance
            gs_audio_source_t* src = gs_slot_array_getp(audio->sources, inst->src.id);

            // Easy out if the instance is not playing currently or the source is invalid
            if (!inst->playing || !src) {
                continue;
            }

            s16* sample_out = (s16*)output;
            s16* samples = (s16*)src->samples;

            u64 samples_to_write = (u64)frame_count;
            f64 sample_volume = inst->volume;

            // Write to channels
            for (u64 write_sample = 0; write_sample < samples_to_write; ++write_sample)
            {
                s32 channels = src->channels;
                f64 start_sample_position = inst->sample_position;
                s16 start_left_sample;
                s16 start_right_sample;

                // Not sure about this line of code...
                f64 target_sample_position = start_sample_position + (f64)channels * (f64)1.f;

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
    }
    ma_mutex_unlock(&ma->lock);
}

gs_result gs_audio_init(gs_audio_i* audio)
{
    // Set user data of audio to be miniaudio data
    audio->user_data = gs_malloc_init(miniaudio_data_t);
    miniaudio_data_t* output = (miniaudio_data_t*)audio->user_data;

    ma_result result = gs_default_val();

      // Init audio device
    // NOTE: Using the default device. Format is floating point because it simplifies mixing.
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID = NULL;  // NULL for the default playback AUDIO.System.device.
    config.playback.format = ma_format_s16;
    config.playback.channels = 2;
    config.capture.pDeviceID = NULL;  // NULL for the default capture AUDIO.System.device.
    config.capture.format = ma_format_s16;
    config.capture.channels = 1;
    config.sampleRate = 44100;
    config.dataCallback = &ma_audio_commit;
    config.pUserData = NULL;

    output->device_config = config;

    if ((ma_device_init(NULL, &output->device_config, &output->device)) != MA_SUCCESS) {
        gs_assert(false);
    }

    if ((ma_device_start(&output->device)) != MA_SUCCESS) {
        gs_assert(false);
    }

    return GS_RESULT_SUCCESS;
}

gs_result gs_audio_shutdown(gs_audio_i* audio)
{
    return GS_RESULT_SUCCESS;
}

#undef GS_AUDIO_IMPL_MINIAUDIO
#endif // GS_AUDIO_IMPL_MINIAUDIO

#endif // __GS_AUDIO_IMPL_H__



















