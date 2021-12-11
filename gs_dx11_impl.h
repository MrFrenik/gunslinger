/*
   This file will contain the actual implementation details for the graphics
   layer, in Direct3D 11. For now we'll follow the GL implementation and try
   to follow a similar approach here as best we can. Big strokes first,
   smaller stuff later.
*/

#ifndef GS_DX11_IMPL_H
#define GS_DX11_IMPL_H

/*=============================
// Headers
=============================*/

#include "gs_dx11.h"
#define CINTERFACE
#define COBJMACROS
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <stdio.h>

/*=============================
// Globals
=============================*/

/*=============================
// Enums
=============================*/

typedef enum gs_dx11_op_code_type
{
	GS_DX11_OP_BEGIN_RENDER_PASS = 0x00,
	GS_DX11_OP_END_RENDER_PASS,
	GS_DX11_OP_CLEAR,
	GS_DX11_OP_COUNT
} gs_dx11_op_code_type;

/*=============================
// Structures
=============================*/

typedef struct _TAG_gsdx11_shader_t
{
	union
	{
		ID3D11VertexShader *vs;
		ID3D11PixelShader *ps;
	};
	ID3DBlob *blob; // need the bytecode for creating input layouts
} gsdx11_shader_t;

typedef ID3D11Buffer	*gsdx11_buffer_t;

// Internal DX11 data
typedef struct _tag_gsdx11_data
{
	gs_slot_array(gsdx11_shader_t)		shaders;
	gs_slot_array(gsdx11_buffer_t)		vertex_buffers;
	gs_slot_array(gsdx11_buffer_t)		index_buffers;

	// Global data that I'll just put here since it's appropriate
	ID3D11Device						*device;
	ID3D11DeviceContext					*context;
	IDXGISwapChain						*swapchain;
	ID3D11RenderTargetView				*rtv;
	ID3D11DepthStencilView				*dsv;
	// NOTE(matthew): putting these here for now, although doing so is probably
	// unnecesary. The raster_state will likely turn into an array of raster
	// states if we add functionality for doing multiple render passes.
	// At the end of the day, we need to set up these two as part of DX11
	// initialization, so might as well save them here.
	ID3D11RasterizerState				*raster_state;
	D3D11_VIEWPORT						viewport;
} gsdx11_data_t;

/*=============================
// Utility Functions
=============================*/

// NOTE(matthew): Putting these here for organization purposes (have them all
// in one place without needing to look back at the ogl implementation). These
// will be filled out on a needs basis, many are not required at the time of
// writing this. Also note that some of these may be invalid for DX11, in
// which case they just get scraped, or we determine their DX11 equivalents
// (ie, no uniforms in DX11... what then??).
int32_t 			gsdx11_buffer_usage_to_dx11_enum(gs_graphics_buffer_usage_type type);
uint32_t		 	gsdx11_access_type_to_dx11_access_type(gs_graphics_access_type type);
uint32_t		 	gsdx11_texture_wrap_to_dx11_texture_wrap(gs_graphics_texture_wrapping_type type);
uint32_t		 	gsdx11_texture_format_to_dx11_data_type(gs_graphics_texture_format_type type);
uint32_t		 	gsdx11_texture_format_to_dx11_texture_format(gs_graphics_texture_format_type type);
uint32_t		 	gsdx11_texture_format_to_dx11_texture_internal_format(gs_graphics_texture_format_type type);
uint32_t		 	gsdx11_shader_stage_to_dx11_stage(gs_graphics_shader_stage_type type);
uint32_t		 	gsdx11_primitive_to_dx11_primitive(gs_graphics_primitive_type type);
uint32_t		 	gsdx11_blend_equation_to_dx11_blend_eq(gs_graphics_blend_equation_type eq);
uint32_t		 	gsdx11_blend_mode_to_dx11_blend_mode(gs_graphics_blend_mode_type type, uint32_t def);
uint32_t		 	gsdx11_cull_face_to_dx11_cull_face(gs_graphics_face_culling_type type);
uint32_t		 	gsdx11_winding_order_to_dx11_winding_order(gs_graphics_winding_order_type type);
uint32_t		 	gsdx11_depth_func_to_dx11_depth_func(gs_graphics_depth_func_type type);
uint32_t		 	gsdx11_stencil_func_to_dx11_stencil_func(gs_graphics_stencil_func_type type);
uint32_t			gsdx11_stencil_op_to_dx11_stencil_op(gs_graphics_stencil_op_type type);
void				gsdx11_uniform_type_to_dx11_uniform_type(gs_graphics_uniform_type gstype); // NOTE(matthew): DX11 doesn't have uniforms!
uint32_t 			gsdx11_index_buffer_size_to_dx11_index_type(size_t sz);
size_t 				gsdx11_get_byte_size_of_vertex_attribute(gs_graphics_vertex_attribute_type type);
size_t 				gsdx11_calculate_vertex_size_in_bytes(gs_graphics_vertex_attribute_desc_t* layout, uint32_t count);
size_t  			gsdx11_get_vertex_attr_byte_offest(gs_dyn_array(gs_graphics_vertex_attribute_desc_t) layout, uint32_t idx);
size_t 				gsdx11_uniform_data_size_in_bytes(gs_graphics_uniform_type type);

/*=============================
// Graphics Initialization
=============================*/

/* // Initialize DX11 specific data (device, context, swapchain  etc.) */
/* void gs_graphics_init(gs_graphics_t *graphics); */

/*=============================
// Resource Creation
=============================*/

/* // Create a vertex buffer */
/* gs_handle(gs_graphics_vertex_buffer_t) gs_graphics_vertex_buffer_create(const gs_graphics_vertex_buffer_desc_t *desc); */

/* // Create an index buffer */
/* gs_handle(gs_graphics_index_buffer_t) gs_graphics_index_buffer_create(const gs_graphics_index_buffer_desc_t* desc) */

/* // Create a shader */
/* gs_handle(gs_graphics_shader_t) gs_graphics_shader_create(const gs_graphics_shader_desc_t *desc); */

/*=============================================================================
// ===== DX11 Implementation =============================================== //
=============================================================================*/

/*=============================
// Utility Functions
=============================*/




/*=============================
// Graphics Initialization
=============================*/

gs_graphics_t *
gs_graphics_create()
{
	gs_graphics_t		*gfx = gs_malloc_init(gs_graphics_t);


	gfx->user_data = gs_malloc_init(gsdx11_data_t);

	return gfx;
}

void
gs_graphics_destroy(gs_graphics_t *graphics)
{
	gsdx11_data_t		*dx11;


	if (!graphics)
		return;

	dx11 = (gsdx11_data_t *)graphics->user_data;

	// free data
	gs_slot_array_free(dx11->shaders);
	gs_slot_array_free(dx11->vertex_buffers);

	gs_free(graphics);
}

// TODO(matthew): create viewport + raster state in here as well
void
gs_graphics_init(gs_graphics_t *graphics)
{
	HRESULT                     hr;
    ID3D11Texture2D             *backbuffer,
                                *ds_buffer;
    DXGI_MODE_DESC              buffer_desc = {0};
    DXGI_SWAP_CHAIN_DESC        swapchain_desc = {0};
    D3D11_TEXTURE2D_DESC        ds_desc = {0};
	gsdx11_data_t				*dx11;
	void						*gs_window;
	HWND						hwnd = 0;
	gsdx11_shader_t				s = {0}; // TODO(matthew): bulletproof this, empty struct for now
    D3D11_RASTERIZER_DESC       raster_state_desc = {0};
    uint32_t 					window_width = gs_engine_subsystem(app).window_width,
    							window_height  = gs_engine_subsystem(app).window_height;


	dx11 = (gsdx11_data_t *)graphics->user_data;
	gs_window = gs_slot_array_get(gs_engine_subsystem(platform)->windows, 1);
	hwnd = glfwGetWin32Window(gs_window);

	// TODO(matthew): could add a 'render targets' field eventually
	gs_slot_array_insert(dx11->shaders, s);
	gs_slot_array_insert(dx11->vertex_buffers, 0);

	// TODO(matthew): we're filling out many of these with predetermined
	// values for now. This is only for development purpsoes. In the future,
	// we'll want to map gunslinger application options to these. For
	// instance, we are forcing the window to be windowed by default; later,
	// we should check the gs_app_desc_t to see if the window should be
	// windowed or fullscreen.
    buffer_desc.Width = window_width;
    buffer_desc.Height = window_height;
    buffer_desc.RefreshRate.Denominator = 1;
    buffer_desc.RefreshRate.Numerator = 60;
    buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapchain_desc.BufferDesc = buffer_desc;
    swapchain_desc.SampleDesc.Count = 1;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.BufferCount = 1;
    swapchain_desc.OutputWindow = hwnd; // TODO: NEED TO GET THIS WINDOW HANDLE
    swapchain_desc.Windowed = TRUE;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    ds_desc.Width = window_width;
    ds_desc.Height = window_height;
    ds_desc.MipLevels = 1;
    ds_desc.ArraySize = 1;
    ds_desc.Format = DXGI_FORMAT_D32_FLOAT;
    ds_desc.SampleDesc.Count = 1;
    ds_desc.Usage = D3D11_USAGE_DEFAULT;
    ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
			D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &swapchain_desc,
			&dx11->swapchain, &dx11->device,NULL, &dx11->context);
    hr = IDXGISwapChain_GetBuffer(dx11->swapchain, 0, &IID_ID3D11Texture2D, (void **)&backbuffer);
    hr = ID3D11Device_CreateRenderTargetView(dx11->device, backbuffer, NULL, &dx11->rtv);
    hr = ID3D11Device_CreateTexture2D(dx11->device, &ds_desc, 0, &ds_buffer);
    hr = ID3D11Device_CreateDepthStencilView(dx11->device, ds_buffer, NULL, &dx11->dsv);
    ID3D11DeviceContext_OMSetRenderTargets(dx11->context, 1, &dx11->rtv, dx11->dsv);

	dx11->viewport.Width = window_width;
	dx11->viewport.Height = window_height;
	dx11->viewport.MaxDepth = 1.0f;
	ID3D11DeviceContext_RSSetViewports(dx11->context, 1, &dx11->viewport);

	raster_state_desc.FillMode = D3D11_FILL_SOLID;
	raster_state_desc.CullMode = D3D11_CULL_NONE;
	hr = ID3D11Device_CreateRasterizerState(dx11->device, &raster_state_desc, &dx11->raster_state);
    ID3D11DeviceContext_RSSetState(dx11->context, dx11->raster_state);
}



/*=============================
// Resource Creation
=============================*/

gs_handle(gs_graphics_vertex_buffer_t)
gs_graphics_vertex_buffer_create(const gs_graphics_vertex_buffer_desc_t *desc)
{
	HRESULT										hr;
	ID3D11Buffer								*buffer;
	D3D11_BUFFER_DESC							buffer_desc = {0};
	D3D11_SUBRESOURCE_DATA						buffer_data = {0};
	gsdx11_data_t								*dx11;
	gs_handle(gs_graphics_vertex_buffer_t)		hndl = gs_default_val();


	dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;
	buffer_desc.ByteWidth = desc->size;
	// TODO(matthew): Later, we need to map more of these fields according to
	// the data in the desc. Will need to create functions that map GS enums
	// to DX11 enums.
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_data.pSysMem = desc->data;

	if (desc->data)
		hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, &buffer_data, &buffer);
	else
		hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, NULL, &buffer);

	hndl = gs_handle_create(gs_graphics_vertex_buffer_t, gs_slot_array_insert(dx11->vertex_buffers, buffer));
	printf("1: %p\r\n", buffer);
	printf("2: %p\n", (gs_slot_array_getp(dx11->vertex_buffers, 1)));

	return hndl;
}

gs_handle(gs_graphics_index_buffer_t)
gs_graphics_index_buffer_create(const gs_graphics_index_buffer_desc_t* desc)
{
	HRESULT										hr;
	ID3D11Buffer								*buffer;
	D3D11_BUFFER_DESC							buffer_desc = {0};
	D3D11_SUBRESOURCE_DATA						buffer_data = {0};
	gsdx11_data_t								*dx11;
	gs_handle(gs_graphics_index_buffer_t)		hndl = gs_default_val();


	dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;
	buffer_desc.ByteWidth = desc->size;
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_data.pSysMem = desc->data;

	if (desc->data)
		hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, &buffer_data, &buffer);
	else
		hr = ID3D11Device_CreateBuffer(dx11->device, &buffer_desc, NULL, &buffer);

	hndl = gs_handle_create(gs_graphics_index_buffer_t, gs_slot_array_insert(dx11->index_buffers, buffer));

	return hndl;
}

gs_handle(gs_graphics_shader_t)
gs_graphics_shader_create(const gs_graphics_shader_desc_t *desc)
{
	HRESULT								hr;
	gsdx11_data_t						*dx11;
	ID3DBlob							*err_blob;
	void								*shader_src = desc->sources[0].source;
	size_t								shader_len = strlen(shader_src) + 1;
	gsdx11_shader_t						shader;
	uint32_t							shader_type = desc->sources[0].type;
	gs_handle(gs_graphics_shader_t)		hndl;


	dx11 = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;
	// TODO(matthew): Check the error blob
	// TODO(matthew): Make this support multiple shader sources eventually
	switch (shader_type)
	{
		case GS_GRAPHICS_SHADER_STAGE_VERTEX:
		{
			hr = D3DCompile(shader_src, shader_len, NULL, NULL, NULL, "main", "vs_5_0",
					0, 0, &shader.blob, &err_blob);
			hr = ID3D11Device_CreateVertexShader(dx11->device, ID3D10Blob_GetBufferPointer(shader.blob),
					ID3D10Blob_GetBufferSize(shader.blob), 0, &shader.vs);
			hndl = gs_handle_create(gs_graphics_shader_t, gs_slot_array_insert(dx11->shaders, shader));
			printf("v: %p\n", shader.vs);
		} break;
		case GS_GRAPHICS_SHADER_STAGE_FRAGMENT:
		{
			hr = D3DCompile(shader_src, shader_len, NULL, NULL, NULL, "main", "ps_5_0",
					0, 0, &shader.blob, &err_blob);
			hr = ID3D11Device_CreatePixelShader(dx11->device, ID3D10Blob_GetBufferPointer(shader.blob),
					ID3D10Blob_GetBufferSize(shader.blob), 0, &shader.ps);
			hndl = gs_handle_create(gs_graphics_shader_t, gs_slot_array_insert(dx11->shaders, shader));
			printf("p: %p\n", shader.ps);
		} break;
	}

	return hndl;
}

void
gs_graphics_shutdown(gs_graphics_t* graphics)
{
}

gs_handle(gs_graphics_texture_t)
gs_graphics_texture_create(const gs_graphics_texture_desc_t* desc)
{
}



/*=============================
// Command Buffers Ops
=============================*/

#define __dx11_push_command(__cb, __op_code, ...)												\
do 																								\
{																								\
	gsdx11_data_t *__gsdx11_data = (gsdx11_data_t *)gs_engine_subsystem(graphics)->user_data;	\
	gs_byte_buffer_write(&__cb->commands, u32, (u32)__op_code);									\
	__VA_ARGS__																					\
	__cb->num_commands++;																		\
} while (0)

void
gs_graphics_begin_render_pass(gs_command_buffer_t *cb,
							  gs_handle(gs_graphics_render_pass_t) hndl)
{
	__dx11_push_command(cb, GS_DX11_OP_BEGIN_RENDER_PASS,
	{
		gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
	});
}

void
gs_graphics_end_render_pass(gs_command_buffer_t *cb)
{
	__dx11_push_command(cb, GS_DX11_OP_END_RENDER_PASS,
	{
		// Nothing...
	});
}

void
gs_graphics_clear(gs_command_buffer_t *cb,
				  gs_graphics_clear_desc_t *desc)
{
}

#endif // GS_DX11_IMPL_H
