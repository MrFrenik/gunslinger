/*
	This file will just hold a few additional DX11 specific things that gs.h
	will need to have at the top level. For instance, gs.h contains a
	'gs_opengl_video_settings_t' struct for containing API context data;
	in the same way I've added a 'gs_dx11_video_settings_t' struct for doing
	the same thing.
	Should only need a couple extra things in this file, the majority of the
	work will go into gs_dx11_impl.h since that's where the graphics layer
	implementation should live as per John's indication.
*/

#ifndef GS_DX11_H
#define GS_DX11_H

#include "gs/gs.h"
/* #define GLFW_EXPOSE_NATIVE_WIN32 */
/* #include "glfw3native.h" */

typedef struct _TAG_gs_dx11_video_settings
{
	uint32_t		major_version,
					minor_version;
	uint8_t			multisampling_count;
	void			*ctx;
} gs_dx11_video_settings_t;

typedef union _TAG_gs_dx11_graphics_api_settings
{
	gs_dx11_video_settings_t 	dx11;
	bool32						debug;
} gs_dx11_graphics_api_settings_t;

#endif // GS_DX11_H
