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

typedef struct gs_application_context
{
	gs_result ( * update )();
	gs_result ( * shutdown )();
} gs_application_context;

// What would the context necessarily hold? Some container of all subsystems? 
typedef struct
{
	struct gs_platform_i* 			platform;		// Main platform interface
	gs_application_context 			app;
} gs_engine_context;

// This could be kept in an implementation file and just provide an interface to the user
typedef struct 
{
	gs_engine_context ctx;
	gs_result ( * run )();
	gs_result ( * shutdown )();
} gs_engine;

// gs_engine* gs_engine_construct();
void gs_engine_init( gs_engine* engine );
gs_engine* gs_engine_instance();

#ifdef __cplusplus
}
#endif 	// c++

#endif // GS_ENGINE_H

