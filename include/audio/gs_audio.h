#ifndef __GS_AUDIO_H__
#define __GS_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "common/gs_containers.h"

typedef enum gs_audio_file_type
{
	gs_ogg = 0x00,
	gs_wav	
} gs_audio_file_type;

/*==================
// Audio Source
==================*/

typedef struct gs_audio_source_t
{
	s32 channels;
	s32 sample_rate;
	void* samples;
	s32 sample_count;
} gs_audio_source_t;

gs_resource_cache_decl(gs_audio_source_t);

/*=====================
// Audio Instance Data
=====================*/

typedef struct gs_audio_instance_data_t
{
	gs_resource(gs_audio_source_t) src;
	f32 volume;
	b32 loop;
	b32 persistent;
	b32 playing;
	f64 sample_position;
	void* user_data;
} gs_audio_instance_data_t;

typedef struct gs_audio_instance_data_t gs_audio_instance_t;

gs_resource_cache_decl(gs_audio_instance_data_t);
gs_resource_cache_decl(gs_audio_instance_t);

/*=====================
// Internal Audio Data
=====================*/

typedef struct gs_audio_data_t
{
	// Samples to actually write to hardware
	void* sample_out;

	// Amount of samples to write
    u32 sample_count_to_output;	

    // Samples per second for hardward
    u32 samples_per_second;

    // All the registered instances to actually play
    gs_resource_cache(gs_audio_instance_t) instance_cache;

    // All registered sources
    gs_resource_cache(gs_audio_source_t) audio_cache;

    // Any internal data required for audio API
    void* internal;

} gs_audio_data_t;

gs_force_inline
gs_audio_instance_data_t gs_audio_instance_data_new(gs_resource(gs_audio_source_t) src)
{
	gs_audio_instance_data_t inst = {0};
	inst.src = src;
	inst.volume = 0.5f;
	inst.loop = false;
	inst.persistent = false;
	inst.playing = true;
	inst.user_data = NULL;
	return inst;
}

typedef struct gs_audio_i
{
	/*============================================================
	// Audio Initilization / De-Initialization
	============================================================*/

	gs_result (* init)(struct gs_audio_i*);
	gs_result (* shutdown)(struct gs_audio_i*);
	gs_result (* update)(struct gs_audio_i*);
	gs_result(* commit)(struct gs_audio_i*);

	/*============================================================
	// Audio Source
	============================================================*/

	gs_resource(gs_audio_source_t) (* load_audio_source_from_file)(const char* file_name);

	/*============================================================
	// Audio Instance Data
	============================================================*/

	gs_resource(gs_audio_instance_t)(* construct_instance)(gs_audio_instance_data_t);
	void (* play_source)(gs_resource(gs_audio_source_t) src, f32 volume);
	void (* play)( gs_resource(gs_audio_instance_t));
	void (* pause)(gs_resource(gs_audio_instance_t));
	void (* stop)(gs_resource(gs_audio_instance_t));
	void (* restart)(gs_resource(gs_audio_instance_t));
	b32 (* is_playing)(gs_resource(gs_audio_instance_t));

	void (* set_instance_data)(gs_resource(gs_audio_instance_t), gs_audio_instance_data_t);
	gs_audio_instance_data_t (* get_instance_data)(gs_resource(gs_audio_instance_t));
	f32 (* get_volume)(gs_resource(gs_audio_instance_t));
	void (* set_volume)(gs_resource(gs_audio_instance_t), f32);

	void (* get_runtime)(gs_resource(gs_audio_source_t) src, s32* minutes, s32* seconds);
	void (* convert_to_runtime)(s32 sample_count, s32 sample_rate, 
		s32 num_channels, s32 position, s32* minutes_out, s32* seconds_out);

	s32 (* get_sample_count)(gs_resource(gs_audio_source_t) src);
	s32 (* get_sample_rate)(gs_resource(gs_audio_source_t) src);
	s32 (* get_num_channels)(gs_resource(gs_audio_source_t) src);

	// Proably 
	f32 max_audio_volume;
	f32 min_audio_volume;

	// All internal API specific data for audio system
	void* data;

	// Any custom user data (for custom API implementations)
	void* user_data;
} gs_audio_i;

// Extern internal functions
extern struct gs_audio_i* __gs_audio_construct();
extern void gs_audio_construct_internal(struct gs_audio_i* audio);
extern void __gs_audio_set_default_functions(struct gs_audio_i* audio);
extern gs_result __gs_audio_update_internal(struct gs_audio_i* audio);
extern void __gs_audio_play_source(gs_resource(gs_audio_source_t) src, f32 volume);
extern void __gs_audio_play(gs_resource(gs_audio_instance_t) inst_h);
extern void __gs_audio_pause(gs_resource(gs_audio_instance_t) inst_h);
extern void __gs_audio_resume(gs_resource(gs_audio_instance_t) inst_h);
extern void __gs_audio_restart(gs_resource(gs_audio_instance_t) inst_h);
extern void __gs_audio_set_instance_data(gs_resource(gs_audio_instance_t) inst_h, gs_audio_instance_data_t data);
extern f32 __gs_audio_get_volume(gs_resource(gs_audio_instance_t) inst_h);
extern void __gs_audio_set_volume(gs_resource(gs_audio_instance_t) inst_h, f32 vol);
extern void __gs_audio_stop(gs_resource(gs_audio_instance_t) inst_h);

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_AUDIO_H__