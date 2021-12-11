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

// TODO(matthew): we need to get the hWnd so that it can be sent to DX11.
// NOTE(matthew): GLFW provides a function, glfwGetWin32Window which
// returns the hWnd of the specified window. Looks like we can just use this.
// However, at some point in the future we may opt to write our own native
// Win32 platform layer to 1) make DX11/12 backends a bit simpler, 2) remove
// the GLFW dependency on Windows.

// TODO(matthew): DX11 is natively a C++ API, this goes against gunslinger's
// design as being a purely C API.
// NOTE(matthew): DX11 also provides a C interface that can be used instead of
// the C++ one. Looks like we just need #define CINTERFACE before #include
// d3d11.h. Will ask John to see if we can use the C++ API, or if we must use
// the C interface.
// Convention doesn't change much. For instance:
// ID3D11Device *Device;
// Device->CreateBuffer(...) ===> ID3D11Device_CreateBuffer(Device, ...);

#ifndef GS_DX11_H
#define GS_DX11_H

#include "gs/gs.h"

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
} gs_graphics_api_settings_t;

#endif // GS_DX11_H
