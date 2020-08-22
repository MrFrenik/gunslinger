#ifndef SS_RENDER_PASS_I
#define SS_RENDER_PASS_I

#include <gs.h>

// Abstract interface for all render passes passes
typedef struct render_pass_i 
{
	void ( * pass )( gs_command_buffer_t* cb, struct render_pass_i* pass, void* paramters );
} render_pass_i;

#endif