#ifndef __GS_ENGINE_H__
#define __GS_ENGINE_H__

/*═█═════════════════════════════════════════════════════════════════════════════════════█═╗
████ ██████╗ ██╗   ██╗███╗   ██╗███████╗██╗     ██╗███╗   ██╗ ██████╗ ███████╗██████╗ ██████═█
█║█	██╔════╝ ██║   ██║████╗  ██║██╔════╝██║     ██║████╗  ██║██╔════╝ ██╔════╝██╔══██╗ ██═████
███	██║  ███╗██║   ██║██╔██╗ ██║███████╗██║     ██║██╔██╗ ██║██║ ███╗█████╗  ██████╔╝ █████═██
╚██	██║   ██║██║   ██║██║╚██╗██║╚════██║██║     ██║██║╚██╗██║██║   ██║██╔══╝  ██╔══██╗ ███ █╝█
█║█	╚██████╔╝╚██████╔╝██║ ╚████║███████║███████╗██║██║ ╚████║╚██████╔╝███████╗██║  ██║ ██═████
████ ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚══════╝╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝ █═█═██
╚═██════════════════════════════════════════════════════════════════════════════════════██═╝*/

#ifdef __cplusplus
extern "C" {
#endif

#include "common/gs_types.h"

// Forward Decl
struct gs_platform_i;
struct gs_graphics_i;
struct gs_audio_i;

typedef struct gs_application_desc_t
{
	gs_result (* init)();
	gs_result (* update)();
	gs_result (* shutdown)();
	const char* window_title;
	u32 window_width;
	u32 window_height;
	u32 window_flags;
	f32 frame_rate;
	b32 enable_vsync;
	b32 is_running;
	void* user_data;
} gs_application_desc_t;

// What would the context necessarily hold? Some container of all subsystems? 
typedef struct gs_engine_context_t
{
	struct gs_platform_i* 			platform;		// Main platform interface
	struct gs_graphics_i* 			graphics;
	struct gs_audio_i* 				audio;
	gs_application_desc_t 			app;
} gs_engine_context_t;

// This could be kept in an implementation file and just provide an interface to the user
typedef struct gs_engine_t
{
	gs_engine_context_t ctx;
	gs_result (* run)();
	gs_result (* shutdown)();
} gs_engine_t;

gs_engine_t* gs_engine_construct(gs_application_desc_t app_desc);
gs_engine_t* gs_engine_instance();

#define gs_engine_subsystem(T)\
	gs_engine_instance()->ctx.##T

#define gs_engine_user_data(T)\
	(T*)(gs_engine_instance()->ctx.app.user_data)

#ifdef __cplusplus
}
#endif 	// c++

#endif // __GS_ENGINE_H__

