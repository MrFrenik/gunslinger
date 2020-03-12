#ifndef GS_ENGINE_H
#define GS_ENGINE_H

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
struct gs_platform_input;
struct gs_platform_window;

typedef struct gs_world_time 
{
	f32 max_fps;
	f32 fps;
	f32 delta_time;
	f32 total_elapsed_time;
} gs_world_time;

typedef struct gs_application_context
{
	gs_result ( * update )();
	gs_result ( * shutdown )();
} gs_application_context;

// What would the context necessarily hold? Some container of all subsystems? 
typedef struct
{
	struct gs_platform_input*		input;			
	struct gs_platform_i* 			platform;		// Main platform interface
	gs_world_time 					time;
	gs_application_context 			app;
} gs_engine_context;

// This could be kept in an implementation file and just provide an interface to the user
typedef struct 
{
	gs_engine_context ctx;
	gs_result ( * run )();
	gs_result ( * shutdown )();
} gs_engine;

gs_engine* gs_engine_construct();
gs_engine* gs_engine_instance();

#ifdef __cplusplus
}
#endif 	// c++

#endif // GS_ENGINE_H

