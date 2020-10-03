#ifndef __GS_AUDIO_H__
#define __GS_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"
#include "common/gs_containers.h"

// Internal audio resource data
gs_declare_resource_type( gs_audio_source );			// Actual resource data
gs_declare_resource_type( gs_audio_instance );			// Used to instance an audio source (for multiple different instances of the same data)

typedef enum gs_audio_file_type
{
	gs_ogg = 0x00,
	gs_wav	
} gs_audio_file_type;

typedef struct gs_audio_source_t
{
	s32 channels;
	s32 sample_rate;
	void* samples;
	s32 sample_count;
} gs_audio_source_t;

typedef struct gs_audio_instance_data_t
{
	gs_audio_source_t* src;
	f32 volume;
	b32 loop;
	b32 persistent;
	b32 playing;
	f64 sample_position;
	void* user_data;						// Any custom user data required for a specific internal/external usage
} gs_audio_instance_data_t;

typedef gs_resource( gs_audio_instance ) gs_handle_audio_instance;

gs_slot_array_decl( gs_audio_instance_data_t );

typedef struct gs_audio_data_t
{
	// Other internal resource data, like audio resources
	void* sample_out;				// Samples to actually write to hardware buffer
    u32 sample_count_to_output;	
    u32 samples_per_second;

    gs_slot_array( gs_audio_instance_data_t ) 	instances; 	// Instanced data

    // Any internal data required for audio API
    void* internal;

} gs_audio_data_t;

_force_inline
gs_audio_instance_data_t gs_audio_instance_data_new( gs_audio_source_t* src )
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
	gs_result ( * init )( struct gs_audio_i* );
	gs_result ( * shutdown )( struct gs_audio_i* );
	gs_result ( * update )( struct gs_audio_i* );
	gs_result( * commit )( struct gs_audio_i* );

	/*============================================================
	// Audio Source
	============================================================*/
	gs_audio_source_t* ( * load_audio_source_from_file )( const char* file_name );

	/*============================================================
	// Audio Instance Data
	============================================================*/
	gs_resource( gs_audio_instance )( * construct_instance )( gs_audio_instance_data_t );
	void ( * play_source )(  gs_audio_source_t* , f32 volume );
	void ( * play )(  gs_resource( gs_audio_instance ) );
	void ( * pause )( gs_resource( gs_audio_instance ) );
	void ( * stop )( gs_resource( gs_audio_instance ) );
	void ( * restart )( gs_resource( gs_audio_instance ) );
	b32 ( * is_playing )( gs_resource( gs_audio_instance ) );

	void ( * set_instance_data )( gs_resource( gs_audio_instance ), gs_audio_instance_data_t );
	gs_audio_instance_data_t ( * get_instance_data )( gs_resource( gs_audio_instance ) );
	f32 ( * get_volume )( gs_resource( gs_audio_instance ) );
	void ( * set_volume )( gs_resource( gs_audio_instance ), f32 );

	void ( * get_runtime )( gs_audio_source_t*, s32* minutes, s32* seconds );
	void ( * convert_to_runtime )( s32 sample_count, s32 sample_rate, 
		s32 num_channels, s32 position, s32* minutes_out, s32* seconds_out );

	// Proably 
	f32 max_audio_volume;
	f32 min_audio_volume;

	// All internal API specific data for audio system
	void* data;
	void* user_data;		// Any custom user data

} gs_audio_i;

// Extern internal functions
extern struct gs_audio_i* __gs_audio_construct();
extern void gs_audio_construct_internal( struct gs_audio_i* audio );
extern void __gs_audio_set_default_functions( struct gs_audio_i* audio );
extern gs_result __gs_audio_update_internal( struct gs_audio_i* audio );
extern b32 __gs_load_ogg_data( const char* file_name, gs_audio_source_t* src );
extern void __gs_audio_play_source( gs_audio_source_t*, f32 volume );
extern void __gs_audio_play( gs_resource( gs_audio_instance ) inst_h );
extern void __gs_audio_pause( gs_resource( gs_audio_instance ) inst_h );
extern void __gs_audio_resume( gs_resource( gs_audio_instance ) inst_h );
extern void __gs_audio_restart( gs_resource( gs_audio_instance ) inst_h );
extern void __gs_audio_set_instance_data( gs_resource( gs_audio_instance ) inst_h, gs_audio_instance_data_t data );
extern f32 __gs_audio_get_volume( gs_resource( gs_audio_instance ) inst_h );
extern void __gs_audio_set_volume( gs_resource( gs_audio_instance ) inst_h, f32 vol );
extern void __gs_audio_stop( gs_resource( gs_audio_instance ) inst_h );

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_AUDIO_H__