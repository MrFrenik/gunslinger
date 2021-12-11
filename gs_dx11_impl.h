/*
   This file will contain the actual implementation details for the graphics
   layer, in Direct3D 11. For now we'll follow the GL implementation and try
   to follow a similar approach here as best we can. Big strokes first,
   smaller stuff later.
*/

#ifndef GS_DX11_IMPL_H
#define GS_DX11_IMPL_H

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

/*=============================
// Headers
=============================*/

#include "gs_dx11.h"
#define CINTERFACE
#define COBJMACROS
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

/*=============================
// Globals
=============================*/

// NOTE(matthew): put these in the global data struct?
ID3D11Device *g_device;
ID3D11DeviceContext *g_context;
IDXGISwapChain *g_swapchain;
ID3D11RenderTargetView *g_rtv;
ID3D11DepthStencilView *g_dsv;

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
	gs_slot_array(gsdx11_shader_t)	shaders;
	gs_slot_array(gsdx11_buffer_t)	vertex_buffers;
} gsdx11_data_t;

/*=============================
// Graphics Initialization
=============================*/

// Initialize DX11 specific data (device, context, swapchain  etc.)
void gs_graphics_init(gs_graphics_t *graphics);

/*=============================
// Resource Creation
=============================*/

/* // Create a vertex buffer */
/* gs_handle(gs_graphics_vertex_buffer_t) gs_graphics_vertex_buffer_create(const gs_graphics_vertex_buffer_desc_t *desc); */

/* // Create a shader */
/* gs_handle(gs_graphics_shader_t) gs_graphics_shader_create(const gs_graphics_shader_desc_t *desc); */

/*=============================================================================
// ===== DX11 Implementation =============================================== //
=============================================================================*/

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

void
gs_graphics_init(gs_graphics_t *graphics)
{
	HRESULT                     hr;
    ID3D11Texture2D             *backbuffer,
                                *ds_buffer;
    DXGI_MODE_DESC              buffer_desc = {0};
    DXGI_SWAP_CHAIN_DESC        swapchain_desc = {0};
    D3D11_TEXTURE2D_DESC        ds_desc = {0};
	gsdx11_data_t				*dx11 = (gsdx11_data_t *)graphics->user_data;
	void						*gs_window = gs_engine_subsystem(platform)->windows;
	HWND						hWnd = 0;
	gsdx11_shader_t				s = {0}; // TODO(matthew): bulletproof this, empty struct for now


	// TODO(matthew): could add a 'render targets' field eventually
	gs_slot_array_insert(dx11->shaders, s);
	gs_slot_array_insert(dx11->vertex_buffers, 0);

    buffer_desc.Width = SCR_WIDTH;
    buffer_desc.Height = SCR_HEIGHT;
    buffer_desc.RefreshRate.Denominator = 1;
    buffer_desc.RefreshRate.Numerator = 60;
    buffer_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    buffer_desc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    buffer_desc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    swapchain_desc.BufferDesc = buffer_desc;
    swapchain_desc.SampleDesc.Count = 1;
    swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchain_desc.BufferCount = 1;
    swapchain_desc.OutputWindow = hWnd; // TODO: NEED TO GET THIS WINDOW HANDLE
    swapchain_desc.Windowed = true;
    swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    ds_desc.Width = SCR_WIDTH;
    ds_desc.Height = SCR_HEIGHT;
    ds_desc.MipLevels = 1;
    ds_desc.ArraySize = 1;
    ds_desc.Format = DXGI_FORMAT_D32_FLOAT;
    ds_desc.SampleDesc.Count = 1;
    ds_desc.Usage = D3D11_USAGE_DEFAULT;
    ds_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
			D3D11_CREATE_DEVICE_DEBUG, 0, 0, D3D11_SDK_VERSION, &swapchain_desc,
			&g_swapchain, &g_device,NULL, &g_context);
    hr = IDXGISwapChain_GetBuffer(g_swapchain, 0, &IID_ID3D11Texture2D, (void **)&backbuffer);
    hr = ID3D11Device_CreateRenderTargetView(g_device, backbuffer, NULL, &g_rtv);
    hr = ID3D11Device_CreateTexture2D(g_device, &ds_desc, 0, &ds_buffer);
    hr = ID3D11Device_CreateDepthStencilView(g_device, ds_buffer, NULL, &g_dsv);
    ID3D11DeviceContext_OMSetRenderTargets(g_context, 1, &g_rtv, g_dsv);
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
	buffer_data.pSysMem = desc->data;

	if (desc->data)
		hr = ID3D11Device_CreateBuffer(g_device, &buffer_desc, &buffer_data, &buffer);
	else
		hr = ID3D11Device_CreateBuffer(g_device, &buffer_desc, NULL, &buffer);

	hndl = gs_handle_create(gs_graphics_vertex_buffer_t, gs_slot_array_insert(dx11->vertex_buffers, buffer));

	return hndl;
}

gs_handle(gs_graphics_shader_t)
gs_graphics_shader_create(const gs_graphics_shader_desc_t *desc)
{
	HRESULT				hr;
	gsdx11_data_t		*dx11;
	ID3DBlob			*err_blob;
	void 				*shader_src = desc->sources[0].source;
	size_t				shader_len = strlen(shader_src) + 1;
	gsdx11_shader_t		shader;
	uint32_t			shader_type = desc->sources[0].type;


	// TODO(matthew): Check the error blob
	// TODO(matthew): Make this support multiple shader sources eventually
	switch (shader_type)
	{
		case GS_GRAPHICS_SHADER_STAGE_VERTEX:
		{
			hr = D3DCompile(shader_src, shader_len, NULL, NULL, NULL, "main", "vs_5_0",
					0, 0, &shader.blob, &err_blob);
			hr = ID3D11Device_CreateVertexShader(g_device, ID3D10Blob_GetBufferPointer(shader.blob),
					ID3D10Blob_GetBufferSize(shader.blob), 0, &shader.vs);
		};
		case GS_GRAPHICS_SHADER_STAGE_FRAGMENT:
		{
			hr = D3DCompile(shader_src, shader_len, NULL, NULL, NULL, "main", "ps_5_0",
					0, 0, &shader.blob, &err_blob);
			hr = ID3D11Device_CreatePixelShader(g_device, ID3D10Blob_GetBufferPointer(shader.blob),
					ID3D10Blob_GetBufferSize(shader.blob), 0, &shader.ps);
		};
	}

	return gs_handle_create(gs_graphics_shader_t, gs_slot_array_insert(dx11->shaders, shader));
}

#endif // GS_DX11_IMPL_H
