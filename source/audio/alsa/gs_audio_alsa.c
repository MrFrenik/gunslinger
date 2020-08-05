#include "audio/gs_audio.h"

#include "base/gs_engine.h"
#include "platform/gs_platform.h"
#include "common/gs_util.h"

gs_result audio_init( struct gs_audio_i* audio )
{
	return gs_result_success;
}

gs_result audio_shutdown( struct gs_audio_i* audio )
{
	return gs_result_success;
}

gs_result audio_commit( struct gs_audio_i* audio )
{
	return gs_result_success;
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
	audio->shutdown 	= &audio_shutdown;
	audio->commit 		= &audio_commit;

	// Default internals
	__gs_audio_set_default_functions( audio );

	return audio;
}
